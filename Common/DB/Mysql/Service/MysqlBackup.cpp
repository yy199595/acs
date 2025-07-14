//
// Created by 64658 on 2025/6/17.
//

#include "MysqlBackup.h"
#include "Server/Config/ServerConfig.h"
#include "Mysql/Component/MysqlDBComponent.h"
#include "Util/File/DirectoryHelper.h"
#include "Util/Tools/TimeHelper.h"
#include "DB/Common/SqlFactory.h"
#include "Util/Tools/String.h"
#include "Util/Tools/Math.h"
#include "Util/Zip/Zip.h"
#include "DB/Common/SqlFactory.h"
#include "Util/File/FileHelper.h"

namespace acs
{
	MysqlBackup::MysqlBackup()
	{
		this->mMysql = nullptr;
		mysql::BackupConfig::RegisterFields();
	}

	bool MysqlBackup::OnInit()
	{
		BIND_COMMON_HTTP_METHOD(MysqlBackup::Backup);
		BIND_COMMON_HTTP_METHOD(MysqlBackup::Recover);
		this->mMysql = this->GetComponent<MysqlDBComponent>();
		return ServerConfig::Inst()->Get("mysql.backup", this->mConfig);
	}

	int MysqlBackup::Backup(const http::FromContent& request, json::w::Document& response)
	{
		std::string name;
		LOG_ERROR_CHECK_ARGS(request.Get("name", name))
		std::string dir = fmt::format("{}/{}", this->mConfig.path, name);
		if (!help::dir::DirectorIsExist(dir))
		{
			help::dir::MakeDir(dir);
		}
		int page = 0;
		int limit = 500;
		sql::Factory sqlFactory;
		std::unique_ptr<json::w::Value> jsonValue = response.AddObject("data");
		for (const std::string& tab: this->mConfig.collections)
		{
			std::ofstream ofs;
			unsigned int doneCount = 0;
			std::string path = fmt::format("{}/{}.json", dir, tab);
			ofs.open(path, std::ios::ate | std::ios::out | std::ios::binary);
			if (!ofs.is_open())
			{
				return XCode::Failure;
			}
			std::unique_ptr<mysql::Response> response1;
			do
			{
				page++;
				sqlFactory.GetTable(tab).Select().Page(page, limit);
				response1 = this->mMysql->Run(sqlFactory.ToString());
				for (const std::string & content : response1->contents)
				{
					++doneCount;
					ofs.write(content.c_str(), content.size()) << '\n';
				}
				LOG_DEBUG("[{}] => {}", tab, doneCount)
				ofs.flush();
			}
			while (response1 != nullptr && !response1->contents.empty());
			jsonValue->Add(tab.c_str(), doneCount);
			ofs.close();
		}
		std::string zipPath = fmt::format("{}/{}.zip", this->mConfig.path, name);
		if(help::zip::Create(dir, zipPath))
		{
			help::dir::RemoveAllFile(dir);
		}
		return XCode::Ok;
	}

	int MysqlBackup::Recover(const http::FromContent& request, json::w::Document& response)
	{
		std::string name, db;
		LOG_ERROR_CHECK_ARGS(request.Get("db", db))
		LOG_ERROR_CHECK_ARGS(request.Get("name", name))
		std::string outDir = fmt::format("{}/{}", this->mConfig.path, name);
		std::string zipPath = fmt::format("{}/{}.zip", this->mConfig.path, name);
		if(!help::zip::Unzip(outDir, zipPath))
		{
			return XCode::Failure;
		}
		sql::Factory sqlFactory;
		std::vector<std::string> filePaths;
		help::dir::GetFilePaths(outDir, filePaths);
		std::unique_ptr<json::w::Value> jsonData = response.AddObject("data");
		for(const std::string & filePath : filePaths)
		{
			std::fstream fs;
			std::string table;
			unsigned int doneCount = 0;
			help::fs::GetFileName(filePath, table);
			fs.open(filePath, std::ios::in | std::ios::binary);
			if (fs.is_open())
			{
				std::string line;
				unsigned int count = 0;
				do
				{
					count = 0;
					std::unique_ptr<mysql::Request> mysqlRequest = std::make_unique<mysql::Request>();
					for(size_t index = 0; index < 100 && std::getline(fs, line); index++)
					{
						++count;
						json::r::Document document;
						if(document.Decode(line.c_str(), line.size()))
						{
							sqlFactory.GetTable(table).Insert(document);
							mysqlRequest->AddBatch(sqlFactory.ToString());
						}
					}
					std::unique_ptr<mysql::Response> mysqlResponse = this->mMysql->Run(mysqlRequest);
					if(mysqlResponse != nullptr && mysqlResponse->IsOk())
					{
						doneCount += mysqlResponse->ok.mAffectedRows;
					}
				}
				while(count > 100);
			}
			jsonData->Add(table.c_str(), doneCount);
		}
		return XCode::Ok;
	}
}