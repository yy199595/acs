//
// Created by mac on 2022/5/19.
//

#ifndef SERVER_MONGOSERVICE_H
#define SERVER_MONGOSERVICE_H
#include"DB/Mongo/MongoProto.h"
#include"Component/RpcService/LocalService.h"
namespace Sentry
{
    class MongoService final : public LocalService
	{
	public:
		MongoService();
	private:
		bool OnStartService(ServiceMethodRegister &methodRegister) final;
    private:
        XCode Insert(const s2s::mongo::insert & request);
        XCode Delete(const s2s::mongo::remove & request);
        XCode Update(const s2s::mongo::update & request);
        XCode SetIndex(const s2s::mongo::index & request);
        XCode Query(const s2s::mongo::query::request & request, s2s::mongo::query::response & response);
        XCode RunCommand(const s2s::mongo::command::request & request, s2s::mongo::command::response & response);
    private:
        std::string mBuffer;
        std::set<std::string> mCounters;
        class MongoRpcComponent * mMongoComponent;
	};
}


#endif //SERVER_MONGOSERVICE_H
