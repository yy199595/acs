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
        XCode AddCounter(const s2s::mongo::add_counter::request & request, s2s::mongo::add_counter::response & response);
    private:
        std::string mBuffer;
		class MongoRpcComponent * mMongoComponent;
	};
}


#endif //SERVER_MONGOSERVICE_H
