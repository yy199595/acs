//
// Created by mac on 2022/6/28.
//

#ifndef SERVER_MONGOCOMPONENT_H
#define SERVER_MONGOCOMPONENT_H
#include"Util/NumberBuilder.h"
#include"DB/Mongo/MongoProto.h"
#include"DB/Mongo/MongoClient.h"
#include"Component/Rpc/RpcTaskComponent.h"

using namespace Mongo;

namespace Sentry
{
	class MongoTask : public IRpcTask<Bson::ReaderDocument>
	{
	public:
		MongoTask(int taskId);
	public:
		int GetTimeout() final { return 0;}
		long long GetRpcId() final { return this->mTaskId; }
		void OnResponse(std::shared_ptr<Bson::ReaderDocument> response) final;
		std::shared_ptr<Bson::ReaderDocument> Await() { return mTask.Await(); }
	private:
		int mTaskId;
		TaskSource<std::shared_ptr<Bson::ReaderDocument>> mTask;
	};
}

namespace Sentry
{
	class MongoComponent : public RpcTaskComponent<Bson::ReaderDocument>, public IStart
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
		std::shared_ptr<Bson::ReaderDocument> Run(std::shared_ptr<MongoRequest> request, int flag = 0);
	private:
		Mongo::Config mConfig;
		TimerComponent * mTimerComponent;
		Util::NumberBuilder<int> mRequestId;
		std::unordered_map<long long, long long> mTimers;
		std::vector<std::shared_ptr<MongoClientContext>> mMongoClients;
	};
}


#endif //SERVER_MONGOCOMPONENT_H
