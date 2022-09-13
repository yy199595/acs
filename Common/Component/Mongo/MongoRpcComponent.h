//
// Created by mac on 2022/6/28.
//

#ifndef SERVER_MONGORPCCOMPONENT_H
#define SERVER_MONGORPCCOMPONENT_H
#include"Util/NumberBuilder.h"
#include"DB/Mongo/MongoProto.h"
#include"DB/Mongo/MongoClient.h"
#include"DB/Mongo/BsonDocument.h"
#include"Util/NumberBuilder.h"
#include"Component/Rpc/RpcTaskComponent.h"

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
	class MongoRpcComponent : public RpcTaskComponent<Mongo::CommandResponse>, public IStart
	{
	public:
		MongoRpcComponent() = default;
		~MongoRpcComponent() = default;
	private:
		bool OnStart() final;
		bool LateAwake() final;
		void OnAddTask(RpcTask task) final;
		void OnDelTask(long long taskId, RpcTask task) final;
	public:
		bool Ping(int index);
        bool SetIndex(const std::string & tab, const std::string & name);
    public:
        void OnClientError(int index, XCode code);
        std::shared_ptr<TcpMongoClient> GetClient(int index = 0);
        const Mongo::Config & GetConfig() const { return this->mConfig;}
		std::shared_ptr<Mongo::CommandResponse> Run(std::shared_ptr<TcpMongoClient> mongoClient, std::shared_ptr<CommandRequest> request);
	 private:
		Mongo::Config mConfig;
		TimerComponent * mTimerComponent;
		Util::NumberBuilder<int, 10> mRequestId;
        class NetThreadComponent * mNetComponent;
        std::vector<std::shared_ptr<TcpMongoClient>> mMongoClients;
    };
}


#endif //SERVER_MONGORPCCOMPONENT_H
