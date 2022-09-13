//
// Created by mac on 2022/5/19.
//

#ifndef SERVER_MONGOSERVICE_H
#define SERVER_MONGOSERVICE_H
#include"Message/db.pb.h"
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
        XCode Insert(const db::mongo::insert & request);
        XCode Delete(const db::mongo::remove & request);
        XCode Update(const db::mongo::update & request);
        XCode SetIndex(const db::mongo::index & request);
        XCode Query(const db::mongo::query::request & request, db::mongo::query::response & response);
        XCode RunCommand(const db::mongo::command::request & request, db::mongo::command::response & response);
    private:
        std::string mBuffer;
        class MongoRpcComponent * mMongoComponent;
        class DataSyncComponent * mSyncRedisComponent;
    };
}


#endif //SERVER_MONGOSERVICE_H
