//
// Created by mac on 2022/5/19.
//

#ifndef SERVER_MONGOSERVICE_H
#define SERVER_MONGOSERVICE_H
#include"Message/db.pb.h"
#include"Client/MongoProto.h"
#include"Service/LocalRpcService.h"
namespace Sentry
{
    class MongoService final : public LocalRpcService
	{
	public:
		MongoService();
	private:
        bool Awake() final;
		bool OnStart() final;
        bool OnClose() final;
    private:
        XCode Insert(const db::mongo::insert & request);
        XCode Delete(const db::mongo::remove & request);
        XCode Update(const db::mongo::update & request);
        XCode SetIndex(const db::mongo::index & request);
        XCode Query(const db::mongo::query::request & request, db::mongo::query::response & response);
        XCode RunCommand(const db::mongo::command::request & request, db::mongo::command::response & response);
    private:
        std::string mBuffer;
        class MongoDBComponent * mMongoComponent;
#ifdef __ENABLE_REDIS__
        class DataSyncComponent * mSyncRedisComponent;
#endif
    };
}


#endif //SERVER_MONGOSERVICE_H
