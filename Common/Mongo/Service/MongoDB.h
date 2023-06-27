//
// Created by mac on 2022/5/19.
//

#ifndef SERVER_MONGOSERVICE_H
#define SERVER_MONGOSERVICE_H
#include"Message/s2s/db.pb.h"
#include"Mongo/Client/MongoProto.h"
#include "Mongo/Config/MongoConfig.h"
#include"Rpc/Service/RpcService.h"
namespace Tendo
{
    class MongoDB final : public RpcService
	{
	public:
		MongoDB();
	private:
		bool Awake() final;
        bool OnInit() final;
		int GetClientHandle(int flag = 0);
    private:
		int Save(const db::mongo::update& request);
		int Insert(const db::mongo::insert & request);
		int Delete(const db::mongo::remove & request);
		int Update(const db::mongo::update & request);
		int SetIndex(const db::mongo::index & request);
		int Find(const db::mongo::find::request & request, db::mongo::find::response & response);
		int FindOne(const db::mongo::find_one::request & request, db::mongo::find_one::response & response);
		int Command(const db::mongo::command::request & request, db::mongo::command::response & response);
    private:
        int UpdateData(const db::mongo::update & request, bool upsert);
    private:
		size_t mIndex;
        std::string mBuffer;
		std::vector<int> mClients;
    	Mongo::MongoConfig mConfig;
        class MongoDBComponent * mMongoComponent;
    };
}


#endif //SERVER_MONGOSERVICE_H
