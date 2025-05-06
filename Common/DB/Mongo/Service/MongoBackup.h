//
// Created by leyi on 2023/11/6.
//

#ifndef APP_MONGOBACKUP_H
#define APP_MONGOBACKUP_H
#include"Yyjson/Object/JsonObject.h"
#include"Http/Service/HttpService.h"
#include "Oss/Config/Config.h"
namespace mongo
{
	struct BackupConfig : public json::Object<BackupConfig>
	{
		int batchSize;
		oss::Config oss;
		std::string path;
		std::string upload; //上传路径
		std::vector<std::string> collections;
	};
}

namespace acs
{
	class MongoBackup final : public HttpService
	{
	public:
		MongoBackup();
	private:
		bool Awake() final;
		bool OnInit() final;
	private:
		int Upload(const http::FromContent & request, json::w::Document & response);
		int Backup(const http::FromContent & request, json::w::Document & response);
		int Recover(const http::FromContent & request, json::w::Document & response);
	private:
		mongo::BackupConfig mConfig;
		class MongoDBComponent* mMongo;
	};
}




#endif //APP_MONGOBACKUP_H
