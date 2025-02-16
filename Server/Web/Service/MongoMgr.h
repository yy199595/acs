//
// Created by leyi on 2023/11/6.
//

#ifndef APP_MONGOMGR_H
#define APP_MONGOMGR_H
#include"Http/Service/HttpService.h"

namespace acs
{
	class MongoMgr final : public HttpService
	{
	public:
		MongoMgr();
	private:
		bool OnInit() final;
	private:
		int Backup(const http::FromContent & request, json::w::Document & response);
		int Recover(const http::FromContent & request, json::w::Document & response);
	private:
		std::string mBackupPath;
		class MongoDBComponent* mMongo;
	};
}




#endif //APP_MONGOMGR_H
