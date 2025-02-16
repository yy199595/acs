//
// Created by leyi on 2023/11/6.
//

#include <ostream>
#include "MongoBackup.h"
#include "Util/Zip/Zip.h"
#include "Core/System/System.h"
#include "Entity/Actor/Server.h"
#include "Server/Config/ServerConfig.h"
#include "Util/File/DirectoryHelper.h"
#include "Util/File/FileHelper.h"
#include "Oss/Component/OssComponent.h"
#include "Async/Component/CoroutineComponent.h"
#include "Mongo/Component/MongoDBComponent.h"
namespace acs
{
	MongoBackup::MongoBackup()
	{
		this->mMongo = nullptr;
		REGISTER_JSON_CLASS_MUST_FIELD(mongo::BackupConfig, oss);
		REGISTER_JSON_CLASS_MUST_FIELD(mongo::BackupConfig, path);
		REGISTER_JSON_CLASS_MUST_FIELD(mongo::BackupConfig, collections);
	}

	bool MongoBackup::Awake()
	{
		return ServerConfig::Inst()->Get("mongo.backup", this->mConfig);
	}

	bool MongoBackup::OnInit()
	{
		BIND_COMMON_HTTP_METHOD(MongoBackup::Backup);
		BIND_COMMON_HTTP_METHOD(MongoBackup::Recover);
		BIND_COMMON_HTTP_METHOD(MongoBackup::Upload);
		LOG_CHECK_RET_FALSE(this->mMongo = this->GetComponent<MongoDBComponent>())
		return true;
	}

	int MongoBackup::Upload(const http::FromContent& request, json::w::Document& response)
	{
		std::string name;
		LOG_ERROR_CHECK_ARGS(request.Get("name", name))
		std::string file = fmt::format("{}.zip", name);
		std::string path = fmt::format("{}/{}", this->mConfig.path, file);
		if(!help::fs::FileIsExist(path))
		{
			return XCode::Failure;
		}
		OssComponent* oss = this->GetComponent<OssComponent>();
		if (oss == nullptr)
		{
			return XCode::Failure;
		}
		std::unique_ptr<oss::Response> ossResponse = oss->Upload(this->mConfig.oss, path, "backup");
		if (ossResponse == nullptr)
		{
			return XCode::Failure;
		}
		if(ossResponse->code == HttpStatus::OK)
		{
			std::remove(path.c_str());
		}
		std::unique_ptr<json::w::Value> jsonValue =  response.AddObject("data");
		{
			jsonValue->Add("url", ossResponse->url);
			jsonValue->Add("message", ossResponse->data);
			jsonValue->Add("status", ossResponse->code);
			jsonValue->Add("status_text", HttpStatusToString(ossResponse->code));

		}
		return XCode::Ok;
	}

	int MongoBackup::GetTableCount(const std::string& tab)
	{
		std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
		{
			int count = 0;
			mongoRequest->GetCollection("count", tab);
			std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
			if (mongoResponse && mongoResponse->Document() && mongoResponse->Document()->Get("n", count))
			{
				return count;
			}
		}
		return 0;
	}

	int MongoBackup::Backup(const http::FromContent& request, json::w::Document& response)
	{
		std::string name;
		LOG_ERROR_CHECK_ARGS(request.Get("name", name))
		std::string dir = fmt::format("{}/{}", this->mConfig.path, name);
		if(!help::dir::DirectorIsExist(dir))
		{
			help::dir::MakeDir(dir);
		}
		int sumCount = 0;
		int doneCount = 0;
		for(const std::string & tab : this->mConfig.collections)
		{
			sumCount += this->GetTableCount(tab);
		}
		long long t1 = help::Time::NowMil();
		std::unique_ptr<json::w::Value> jsonValue = response.AddObject("data");
		for(const std::string & tab : this->mConfig.collections)
		{
			std::ofstream ofs;
			std::string path = fmt::format("{}/{}.json", dir, tab);
			ofs.open(path, std::ios::ate | std::ios::out | std::ios::binary);
			if (!ofs.is_open())
			{
				return XCode::Failure;
			}
			int skip = 0;
			int page = 0;
			const int limit = 200;
			unsigned int count = 0;
			std::string jsonString;
			std::unique_ptr<bson::Reader::Document> document1;
			std::vector<std::unique_ptr<bson::Reader::Document>> results;
			do
			{
				page++;
				results.clear();
				std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
				{
					bson::Writer::Document document;
					mongoRequest->GetCollection("find", tab).Skip(skip).Limit(limit);
				}
				std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
				if (mongoResponse == nullptr || mongoResponse->Document() == nullptr)
				{
					break;
				}

				if (mongoResponse->Document()->Get("cursor", document1) && document1->Get("firstBatch", results))
				{
					for (std::unique_ptr<bson::Reader::Document>& document: results)
					{
						++count;
						++doneCount;
						document->WriterToJson(&jsonString);
						ofs.write(jsonString.c_str(), (int)jsonString.size()) << "\n";
					}
					skip += results.size();
					double process = (doneCount / (double)sumCount) * 100;
					LOG_DEBUG("({}) [{:.2f}%] {}/{}", tab, process, doneCount, sumCount);
				}
			} while (results.size() > 0);
			jsonValue->Add(tab.c_str(), count);
			ofs.close();
		}
		long long t2 = help::Time::NowMil();
		std::string zipPath = fmt::format("{}/{}.zip", this->mConfig.path, name);
		if(help::zip::Create(dir, zipPath))
		{
			help::dir::RemoveAllFile(dir);
		}
		long long t3 = help::Time::NowMil();
		LOG_INFO("压缩时间 [{}ms]", t3 - t2);
		LOG_INFO("[{:.2f}s]备份数据成功 => {}", (t2 - t1) / 1000.0f, doneCount)
		return XCode::Ok;
	}

	int MongoBackup::Recover(const http::FromContent& request, json::w::Document& response)
	{
		std::string name, db;
		std::vector<std::string> filePaths;
		std::vector<std::string> fileLines;
		LOG_ERROR_CHECK_ARGS(request.Get("db", db))
		LOG_ERROR_CHECK_ARGS(request.Get("name", name))
		std::string outDir = fmt::format("{}/{}", this->mConfig.path, name);
		std::string zipPath = fmt::format("{}/{}.zip", this->mConfig.path, name);
		if(!help::fs::FileIsExist(zipPath))
		{
			OssComponent* oss = this->GetComponent<OssComponent>();
			if(oss == nullptr)
			{
				return XCode::Failure;
			}
			std::string objectKey = fmt::format("backup/{}.zip", name);
			if(!oss->Download(this->mConfig.oss, objectKey, zipPath))
			{
				std::remove(zipPath.c_str());
				return XCode::Failure;
			}
		}
		help::zip::Unzip(outDir, zipPath);
		help::dir::GetFilePaths(outDir, filePaths);
		std::unique_ptr<json::w::Value> jsonValue = response.AddObject("data");
		{
			for (const std::string& filePath: filePaths)
			{
				int sumCount = 0;
				fileLines.clear();
				std::string table;
				help::fs::GetFileName(filePath, table);
				help::fs::ReadTxtFile(filePath, fileLines);
				std::string targetTable = fmt::format("{}.{}", db, table);
				for (size_t index = 0; index < fileLines.size(); index += 100)
				{
					std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
					{
						bson::Writer::Array documents;
						for (size_t x = index; x < index + 100 && x < fileLines.size(); x++)
						{
							bson::Writer::Document document;
							if (!document.FromByJson(fileLines[x]))
							{
								return XCode::Failure;
							}
							documents.Add(document);
						}
						mongoRequest->GetCollection("insert", targetTable).Insert(documents);
					}
					std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
					if (mongoResponse && mongoResponse->Document() != nullptr)
					{
						int count = 0;
						if (mongoResponse->Document()->Get("n", count))
						{
							sumCount += count;
						}
					}
				}
				jsonValue->Add(targetTable.c_str(), sumCount);
			}
		}
		help::dir::RemoveAllFile(outDir);
		return XCode::Ok;
	}
}