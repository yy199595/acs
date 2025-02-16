//
// Created by leyi on 2023/11/6.
//

#include<ostream>
#include"MongoMgr.h"
#include"Entity/Actor/Server.h"
#include"Core/System/System.h"
#include"Util/Tools/TimeHelper.h"
#include"Mongo/Component/MongoDBComponent.h"
#include"Server/Config/ServerConfig.h"
#include"Util/File/DirectoryHelper.h"
#include "Util/File/FileHelper.h"

namespace acs
{
	MongoMgr::MongoMgr()
	{
		this->mMongo = nullptr;
	}

	bool MongoMgr::OnInit()
	{
		BIND_COMMON_HTTP_METHOD(MongoMgr::Backup);
		BIND_COMMON_HTTP_METHOD(MongoMgr::Recover);
		LOG_CHECK_RET_FALSE(this->mMongo = this->GetComponent<MongoDBComponent>())
		LOG_CHECK_RET_FALSE(ServerConfig::Inst()->GetPath("backup", this->mBackupPath))
		return true;
	}

	int MongoMgr::Backup(const http::FromContent& request, json::w::Document& response)
	{
		std::string tab, name;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("name", name))
		if(this->mBackupPath.empty())
		{
			return XCode::Failure;
		}
		std::string dir = fmt::format("{}/{}", this->mBackupPath, name);
		if(!help::dir::DirectorIsExist(dir))
		{
			help::dir::MakeDir(dir);
		}
		std::ofstream ofs;
		std::string path = fmt::format("{}/{}.mb", dir, tab);
		ofs.open(path, std::ios::ate | std::ios::app | std::ios::out | std::ios::binary);
		if(!ofs.is_open())
		{
			return XCode::Failure;
		}

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
			int skip = (page - 1) * limit;
			std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
			{
				bson::Writer::Document document;
				mongoRequest->GetCollection("find", tab);
			}
			mongoRequest->Skip(skip).Limit(limit);
			std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
			LOG_ERROR_RETURN_CODE(mongoResponse && mongoResponse->Document(), XCode::Failure);

			LOG_ERROR_RETURN_CODE(mongoResponse->Document()->Get("cursor", document1), XCode::FindMongoDocumentFail);
			LOG_ERROR_RETURN_CODE(document1->Get("firstBatch", results), XCode::FindMongoDocumentFail);
			for (std::unique_ptr<bson::Reader::Document>& document: results)
			{
				++count;
				document->WriterToJson(&jsonString);
				ofs.write(jsonString.c_str(), (int)jsonString.size()) << "\n";
			}
		}
		while(results.size() > 0);

		ofs.close();
		std::unique_ptr<json::w::Value> jsonValue = response.AddObject("data");
		{
			jsonValue->Add("count", count);
			jsonValue->Add("name", name);
		}
		return XCode::Ok;
	}

	int MongoMgr::Recover(const http::FromContent& request, json::w::Document& response)
	{
		std::string name, db;
		std::vector<std::string> filePaths;
		std::vector<std::string> fileLines;
		LOG_ERROR_CHECK_ARGS(request.Get("db", db))
		LOG_ERROR_CHECK_ARGS(request.Get("name", name))
		std::unique_ptr<json::w::Value> jsonValue = response.AddObject("data");
		help::dir::GetFilePaths(fmt::format("{}/{}", this->mBackupPath, name), filePaths);
		for(const std::string & filePath : filePaths)
		{
			int sumCount = 0;
			fileLines.clear();
			std::string table;
			help::fs::GetFileName(filePath, table);
			help::fs::ReadTxtFile(filePath, fileLines);
			std::string targetTable = fmt::format("{}.{}", db, table);
			for(size_t index = 0; index < fileLines.size(); index += 100)
			{
				std::unique_ptr<mongo::Request> mongoRequest = std::make_unique<mongo::Request>();
				{
					bson::Writer::Array documents;
					for(size_t x = index; x < index + 100 && x < fileLines.size(); x++)
					{
						bson::Writer::Document document;
						if(!document.FromByJson(fileLines[x]))
						{
							return XCode::Failure;
						}
						documents.Add(document);
					}

					mongoRequest->GetCollection("insert", targetTable).Insert(documents);
				}
				std::unique_ptr<mongo::Response> mongoResponse = this->mMongo->Run(std::move(mongoRequest));
				if(mongoResponse && mongoResponse->Document() != nullptr)
				{
					int count = 0;
					if (mongoResponse->Document()->Get("n", count))
					{
						sumCount+= count;
					}
				}
			}
			jsonValue->Add(targetTable.c_str(), sumCount);
		}

		return XCode::Ok;
	}
}