//
// Created by leyi on 2023/11/6.
//

#ifndef APP_MONGOMGR_H
#define APP_MONGOMGR_H
#include"Http/Service/HttpService.h"

namespace acs
{
	class MongoMgr : public HttpService
	{
	public:
		MongoMgr();
	private:
		bool OnInit() final;
	private:
		int Export(const http::FromContent& request, http::Response& response);
	private:
		std::string mLogDir;
		class MongoComponent* mMongo;
	};
}




#endif //APP_MONGOMGR_H
