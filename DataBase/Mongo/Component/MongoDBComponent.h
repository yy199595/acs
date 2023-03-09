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
    class MongoDBComponent : public RpcTaskComponent<int,Mongo::CommandResponse>,
							 public IStart, public IRpc<Mongo::CommandResponse>
	{
	public:
		MongoDBComponent();
		~MongoDBComponent() final = default;
	private:
		void OnDelTask(int taskId, RpcTask task) final;
	public:
        bool Start() final;
        void CloseClients();
        bool Ping(int index);
        bool SetIndex(const std::string & tab, const std::string & name);
    public:
        void OnClientError(int index, int code);
        TcpMongoClient * GetClient(int index = -1);
		unsigned int GetWaitCount() const { return this->mWaitCount; }
        void Send(TcpMongoClient * mongoClient, std::shared_ptr<CommandRequest> request);
		std::shared_ptr<Mongo::CommandResponse> Run(TcpMongoClient * mongoClient, std::shared_ptr<CommandRequest> request);
	 private:
		void OnConnectSuccessful(const std::string &address) final;
		void OnMessage(std::shared_ptr<CommandResponse> message) final;
	 private:
		 unsigned int mWaitCount;
		Util::NumberBuilder<int, 10> mRequestId;
        std::vector<std::shared_ptr<TcpMongoClient>> mMongoClients;
    };
}


#endif //SERVER_MONGORPCCOMPONENT_H
