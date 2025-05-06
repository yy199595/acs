//
// Created by 64658 on 2025/4/23.
//

#ifndef APP_MONGOREADPROXY_H
#define APP_MONGOREADPROXY_H
#include "Rpc/Service/RpcService.h"
namespace acs
{
	class MongoReadProxy : public RpcService
	{
	public:
		MongoReadProxy();
	private:
		bool OnInit() final;
	private:
		int Find(const json::r::Document & request, json::w::Document & response);
		int Count(const json::r::Document & request, json::w::Document & response);
		int FindOne(const json::r::Document & request, json::w::Document & response);
		int GetMore(const json::r::Document & request, json::w::Document & response);
		int FindPage(const json::r::Document & request, json::w::Document & response);
	private:
		class MongoDBComponent * mMongo;
	};
}


#endif //APP_MONGOREADPROXY_H
