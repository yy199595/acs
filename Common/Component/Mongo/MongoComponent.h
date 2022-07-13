//
// Created by mac on 2022/6/28.
//

#ifndef SERVER_MONGOCOMPONENT_H
#define SERVER_MONGOCOMPONENT_H
#include"Util/NumberBuilder.h"
#include"DB/Mongo/MongoProto.h"
#include"DB/Mongo/MongoClient.h"
#include"DB/Mongo/BsonDocument.h"
#include"Component/Rpc/RpcTaskComponent.h"

using namespace Mongo;

namespace Sentry
{
	class MongoTask : public IRpcTask<Mongo::MongoQueryResponse>
	{
	public:
		MongoTask(int taskId);
	public:
		int GetTimeout() final { return 0;}
		long long GetRpcId() final { return this->mTaskId; }
		void OnResponse(std::shared_ptr<Mongo::MongoQueryResponse> response) final;
		std::shared_ptr<Mongo::MongoQueryResponse> Await() { return mTask.Await(); }
	private:
		int mTaskId;
		TaskSource<std::shared_ptr<Mongo::MongoQueryResponse>> mTask;
	};
}

namespace Sentry
{
	class MongoComponent : public RpcTaskComponent<Mongo::MongoQueryResponse>, public IStart
	{
	public:
		MongoComponent() = default;
		~MongoComponent() = default;
	private:
		bool OnStart() final;
		bool LateAwake() final;
		void OnAddTask(RpcTask task) final;
		void OnDelTask(long long taskId, RpcTask task) final;
	public:
		bool Ping();
		bool InsertOnce(const std::string & tab, const std::string & json);
        bool Delete(const std::string & tab, const std::string & json, int limit);
        std::shared_ptr<Mongo::MongoQueryResponse> Query(const std::string & tab, const std::string & json, int limit = 1);
        bool Update(const std::string & tab, const std::string & update, const std::string & selector, const std::string & tag);
    public:
		void OnClientError(int index, XCode code);
		std::shared_ptr<Mongo::MongoQueryResponse> Run(std::shared_ptr<MongoQueryRequest> request, int flag = 0);
	 private:
		Mongo::Config mConfig;
		TimerComponent * mTimerComponent;
		Util::NumberBuilder<int, 10> mRequestId;
		std::unordered_map<long long, long long> mTimers;
		std::vector<std::shared_ptr<MongoClientContext>> mMongoClients;
	};
}


#endif //SERVER_MONGOCOMPONENT_H
