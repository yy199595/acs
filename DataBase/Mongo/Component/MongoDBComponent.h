//
// Created by mac on 2022/6/28.
//

#ifndef SERVER_MONGORPCCOMPONENT_H
#define SERVER_MONGORPCCOMPONENT_H
#include"Guid/NumberBuilder.h"
#include"Client/MongoProto.h"
#include"Client/MongoClient.h"
#include"Client/BsonDocument.h"
#include"Guid/NumberBuilder.h"
#include"Component/RpcTaskComponent.h"

using namespace Mongo;

namespace Sentry
{
	class MongoTask : public IRpcTask<Mongo::CommandResponse>
	{
	public:
		MongoTask(int taskId, int ms);
	public:
		long long GetRpcId() final { return this->mTaskId; }

    public:
        void OnTimeout() final;
		void OnResponse(std::shared_ptr<Mongo::CommandResponse> response) final;
		std::shared_ptr<Mongo::CommandResponse> Await() { return mTask.Await(); }
	private:
        int mTaskId;
		TaskSource<std::shared_ptr<Mongo::CommandResponse>> mTask;
	};
}


namespace Sentry
{
	class MongoDBComponent : public RpcTaskComponent<Mongo::CommandResponse>
	{
	public:
		MongoDBComponent() = default;
		~MongoDBComponent() = default;
	private:
        bool LoadConfig();
		void OnAddTask(RpcTask task) final;
		void OnDelTask(long long taskId, RpcTask task) final;
	public:
        void CloseClients();
        bool Ping(int index);
        bool StartConnectMongo();
        bool SetIndex(const std::string & tab, const std::string & name);
    public:
        void OnClientError(int index, XCode code);
        TcpMongoClient * GetClient(int index = -1);
        const Mongo::Config & GetConfig() const { return this->mConfig;}
        void Send(TcpMongoClient * mongoClient, std::shared_ptr<CommandRequest> request);
		std::shared_ptr<Mongo::CommandResponse> Run(TcpMongoClient * mongoClient, std::shared_ptr<CommandRequest> request);
	 private:
		Mongo::Config mConfig;
		TimerComponent * mTimerComponent;
		Util::NumberBuilder<int, 10> mRequestId;
        class NetThreadComponent * mNetComponent;
        std::vector<std::shared_ptr<TcpMongoClient>> mMongoClients;
    };
}


#endif //SERVER_MONGORPCCOMPONENT_H
