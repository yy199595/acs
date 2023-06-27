//
// Created by mac on 2022/6/28.
//

#ifndef SERVER_MONGORPCCOMPONENT_H
#define SERVER_MONGORPCCOMPONENT_H
#include"Util/Guid/NumberBuilder.h"
#include"Mongo/Client/MongoProto.h"
#include"Mongo/Client/MongoClient.h"
#include"Mongo/Client/BsonDocument.h"
#include"Util/Guid/NumberBuilder.h"
#include"Mongo/Config/MongoConfig.h"
#include"Rpc/Component/RpcTaskComponent.h"

namespace Tendo
{
	class MongoTask : public IRpcTask<Mongo::CommandResponse>
	{
	public:
		explicit MongoTask(int taskId);
    public:
		void OnResponse(std::shared_ptr<Mongo::CommandResponse> response) final;
		std::shared_ptr<Mongo::CommandResponse> Await() { return mTask.Await(); }
	private:
		TaskSource<std::shared_ptr<Mongo::CommandResponse>> mTask;
	};
}


namespace Tendo
{
    class MongoDBComponent : public RpcTaskComponent<int,Mongo::CommandResponse>, 
							 public IRpc<Mongo::CommandResponse>, public IStart, public ISecondUpdate, public IDestroy
	{
	public:
		MongoDBComponent();
		~MongoDBComponent() final = default;
	public:
		unsigned int GetWaitCount() const { return this->mWaitCount; }
		int MakeMongoClient(const Mongo::MongoConfig & config, bool async = true);
		std::shared_ptr<Mongo::CommandResponse> Run(int id, const std::shared_ptr<Mongo::CommandRequest>& request);
	private:
		std::shared_ptr<Mongo::CommandResponse> SyncRun(int id, const std::shared_ptr<Mongo::CommandRequest>& request);
		std::shared_ptr<Mongo::CommandResponse> AsyncRun(int id, const std::shared_ptr<Mongo::CommandRequest>& request);
	private:
		void Ping();
		void Start() final;
		void OnDestroy() final;
		void OnSecondUpdate(int tick) final;
		void OnConnectSuccessful(const std::string &address) final;
		void OnMessage(std::shared_ptr<Mongo::CommandResponse> message) final;
	private:
    	int mIndex;
		unsigned int mWaitCount;
        std::unordered_map<int, std::shared_ptr<Mongo::TcpMongoClient>> mMongoClients;
		std::unordered_map<int, std::shared_ptr<Mongo::TcpMongoClient>> mSyncMongoClients;
	};
}


#endif //SERVER_MONGORPCCOMPONENT_H
