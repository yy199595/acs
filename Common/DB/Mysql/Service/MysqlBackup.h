//
// Created by 64658 on 2025/6/17.
//

#ifndef APP_MYSQLBACKUP_H
#define APP_MYSQLBACKUP_H
#include "Http/Service/HttpService.h"
#include "Yyjson/Object/JsonObject.h"
#include "AliCloud/Config/Config.h"

namespace mysql
{
	struct BackupConfig : public json::Object<BackupConfig>
	{
		oss::Config oss;
		std::string path;
		std::vector<std::string> collections;

	public:
		static void RegisterFields()
		{
			REGISTER_JSON_CLASS_MUST_FIELD(mysql::BackupConfig, oss);
			REGISTER_JSON_CLASS_MUST_FIELD(mysql::BackupConfig, path);
			REGISTER_JSON_CLASS_MUST_FIELD(mysql::BackupConfig, collections);
		}
	};
}

namespace acs
{
	class MysqlBackup : public HttpService
	{
	public:
		MysqlBackup();
	private:
		bool OnInit() final;
	private:
		int Backup(const http::FromContent & request, json::w::Document & response);
		int Recover(const http::FromContent & request, json::w::Document & response);
	private:
		mysql::BackupConfig mConfig;
		class MysqlDBComponent * mMysql;
	};
}


#endif //APP_MYSQLBACKUP_H
