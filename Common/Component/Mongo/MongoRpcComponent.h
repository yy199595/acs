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
	class MongoTask : public IRpcTask<Mongo::MongoQueryResponse>
	{
	public:
		MongoTask(int taskId, int ms);
	public:
		long long GetRpcId() final { return this->mTaskId; }

    public:
        void OnTimeout() final;
		void OnResponse(std::shared_ptr<Mongo::MongoQueryResponse> response) final;
		std::shared_ptr<Mongo::MongoQueryResponse> Await() { return mTask.Await(); }
	private:
        int mTaskId;
		TaskSource<std::shared_ptr<Mongo::MongoQueryResponse>> mTask;
	};
}


namespace Sentry
{
	class MongoRpcComponent : public RpcTaskComponent<Mongo::MongoQueryResponse>, public IStart
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
		bool InsertOnce(const std::string & tab, const std::string & json);
        bool Delete(const std::string & tab, const std::string & json, int limit);
        std::shared_ptr<Mongo::MongoQueryResponse> Query(const std::string & tab, const std::string & json, int limit = 1);
        bool Update(const std::string & tab, const std::string & update, const std::string & selector, const std::string & tag);
    public:
        void SelectMongoClient(int index);
        void OnClientError(int index, XCode code);
        std::shared_ptr<MongoClientContext> GetClient(int index = 0);
		std::shared_ptr<Mongo::MongoQueryResponse> Run(std::shared_ptr<MongoClientContext> mongoClient, std::shared_ptr<MongoQueryRequest> request);
	 private:
		Mongo::Config mConfig;
		TimerComponent * mTimerComponent;
		Util::NumberBuilder<int, 10> mRequestId;
        class NetThreadComponent * mNetComponent;
        std::shared_ptr<MongoClientContext> mCurClient;
        std::vector<std::shared_ptr<MongoClientContext>> mMongoClients;
    };
}


#endif //SERVER_MONGORPCCOMPONENT_H
