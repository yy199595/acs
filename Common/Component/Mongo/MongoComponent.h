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
	class MongoTask : public IRpcTask<Bson::Read::Object>
	{
	public:
		MongoTask(int taskId);
	public:
		int GetTimeout() final { return 0;}
		long long GetRpcId() final { return this->mTaskId; }
		void OnResponse(std::shared_ptr<Bson::Read::Object> response) final;
		std::shared_ptr<Bson::Read::Object> Await() { return mTask.Await(); }
	private:
		int mTaskId;
		TaskSource<std::shared_ptr<Bson::Read::Object>> mTask;
	};
}

namespace Sentry
{
	class MongoComponent : public RpcTaskComponent<Bson::Read::Object>, public IStart
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
		bool DeleteOnce(const std::string & tab, const std::string & json);
		bool InsertOnce(const std::string & tab, const std::string & json);
		bool UpdateOnce(const std::string & tab, const std::string & update, const std::string & selector);
		std::shared_ptr<Bson::Read::Object> QueryOnce(const std::string & tab, const std::string & json);
	 public:
		void OnClientError(int index, XCode code);
		std::shared_ptr<Bson::Read::Object> Run(std::shared_ptr<MongoQueryRequest> request, int flag = 0);
	 private:
		Mongo::Config mConfig;
		TimerComponent * mTimerComponent;
		Util::NumberBuilder<int, 10> mRequestId;
		std::unordered_map<long long, long long> mTimers;
		std::vector<std::shared_ptr<MongoClientContext>> mMongoClients;
	};
}


#endif //SERVER_MONGOCOMPONENT_H
