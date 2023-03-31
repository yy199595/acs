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

using namespace Mongo;

namespace Sentry
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


namespace Sentry
{
    class MongoDBComponent : public RpcTaskComponent<int,Mongo::CommandResponse>, public IRpc<Mongo::CommandResponse>
	{
	public:
		MongoDBComponent();
		~MongoDBComponent() final = default;
	public:
        void CloseClients();
        bool Ping(int index);
        bool SetIndex(const std::string & tab, const std::string & name);
		inline const MongoConfig & Config() const { return this->mConfig; }
    public:
		int MakeMongoClient();
        void OnClientError(int index, int code);
        TcpMongoClient * GetClient(int index = -1);
		unsigned int GetWaitCount() const { return this->mWaitCount; }
        void Send(TcpMongoClient * mongoClient, std::shared_ptr<CommandRequest> request);
		std::shared_ptr<Mongo::CommandResponse> Run(TcpMongoClient * mongoClient, const std::shared_ptr<CommandRequest>& request);
	 private:
		bool Awake() final;
		void OnConnectSuccessful(const std::string &address) final;
		void OnMessage(std::shared_ptr<CommandResponse> message) final;
		void OnTaskComplete(int key) final { this->mRequestId.Push(key); }
	private:
		MongoConfig mConfig;
		unsigned int mWaitCount;
		Util::NumberBuilder<int, 10> mRequestId;
        std::vector<std::shared_ptr<TcpMongoClient>> mMongoClients;
    };
}


#endif //SERVER_MONGORPCCOMPONENT_H
