//
// Created by 64658 on 2025/4/23.
//

#ifndef APP_MONGOWRITEPROXY_H
#define APP_MONGOWRITEPROXY_H
#include "Rpc/Service/RpcService.h"
namespace acs
{
	class MongoWriteProxy : public RpcService
	{
	public:
		MongoWriteProxy();
	private:
		bool OnInit() final;
	private:
		int SetIndex(const json::r::Document & request);
		int Inc(const json::r::Document & request, json::w::Document & response);
		int Save(const json::r::Document & request, json::w::Document & response);
		int Update(const json::r::Document & request, json::w::Document & response);
		int Insert(const json::r::Document & request, json::w::Document & response);
		int Delete(const json::r::Document & request, json::w::Document & response);
		int Deletes(const json::r::Document & request, json::w::Document & response);
		int Updates(const json::r::Document & request, json::w::Document & response);
		int FindModify(const json::r::Document & request, json::w::Document & response);
	private:
		class MongoDBComponent * mMongo;
	};
}



#endif //APP_MONGOWRITEPROXY_H
