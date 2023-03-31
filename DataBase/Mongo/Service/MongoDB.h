//
// Created by mac on 2022/5/19.
//

#ifndef SERVER_MONGOSERVICE_H
#define SERVER_MONGOSERVICE_H
#include"Message/db.pb.h"
#include"Mongo/Client/MongoProto.h"
#include"Rpc/Service/PhysicalRpcService.h"
namespace Sentry
{
    class MongoDB final : public PhysicalRpcService
	{
	public:
		MongoDB();
	private:
		bool Awake() final;
        bool OnInit() final;
		bool OnStart() final;
        void OnClose() final;
    private:
		int Insert(const db::mongo::insert & request);
		int Delete(const db::mongo::remove & request);
		int Update(const db::mongo::update & request);
		int SetIndex(const db::mongo::index & request);
		int Query(const db::mongo::query::request & request, db::mongo::query::response & response);
		int RunCommand(const db::mongo::command::request & request, db::mongo::command::response & response);
    private:
        std::string mBuffer;
        class MongoDBComponent * mMongoComponent;
    };
}


#endif //SERVER_MONGOSERVICE_H
