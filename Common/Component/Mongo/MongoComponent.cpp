//
// Created by mac on 2022/6/28.
//

#include "MongoComponent.h"
#include"Component/Scene/NetThreadComponent.h"
namespace Sentry
{
	MongoTask::MongoTask(int taskId)
		: mTaskId(taskId)
	{

	}

	void MongoTask::OnResponse(std::shared_ptr<_bson::bsonobj> response)
	{
		this->mTask.SetResult(response);
	}
}

namespace Sentry
{
	bool MongoComponent::LateAwake()
	{
		const ServerConfig & config = this->GetApp()->GetConfig();
		this->mTimerComponent = this->GetApp()->GetTimerComponent();
		LOG_CHECK_RET_FALSE(config.GetMember("mongo", "ip", this->mConfig.mIp));
		LOG_CHECK_RET_FALSE(config.GetMember("mongo", "port", this->mConfig.mPort));
		LOG_CHECK_RET_FALSE(config.GetMember("mongo", "user", this->mConfig.mUser));
		LOG_CHECK_RET_FALSE(config.GetMember("mongo", "passwd", this->mConfig.mPasswd));
		LOG_CHECK_RET_FALSE(config.GetMember("mongo", "count", this->mConfig.mMaxCount));
		return true;
	}

	bool MongoComponent::OnStart()
	{
		for(int index = 0; index < this->mConfig.mMaxCount; index++)
		{
#ifdef ONLY_MAIN_THREAD
			IAsioThread & asioThread = this->GetApp()->GetTaskScheduler();
#else
			NetThreadComponent * netThreadComponent = this->GetComponent<NetThreadComponent>();
			IAsioThread & asioThread = netThreadComponent->AllocateNetThread();
#endif
			std::shared_ptr<SocketProxy> socketProxy =
				std::make_shared<SocketProxy>(asioThread, this->mConfig.mIp, this->mConfig.mPort);
			std::shared_ptr<MongoClientContext> mongoClientContext =
				std::make_shared<MongoClientContext>(socketProxy, this->mConfig, this);
			this->mMongoClients.emplace_back(mongoClientContext);
		}
		std::shared_ptr<MongoQueryRequest> mongoRequest(new MongoQueryRequest());

		mongoRequest->flag = 0;
		mongoRequest->numberToSkip = 0;
		mongoRequest->numberToReturn = 1;
		mongoRequest->collectionName = "admin.$cmd";
		mongoRequest->document.Add("ismaster", 1);
		std::shared_ptr<_bson::bsonobj> mongoResponse = this->Run(mongoRequest);
		std::set<std::string> files;
		mongoResponse->getFieldNames(files);
		return mongoResponse != nullptr && mongoResponse->getField("ismaster").Bool();
	}

	void MongoComponent::OnDelTask(long long taskId, RpcTask task)
	{
		this->mRequestId.Push((int)taskId);
	}

	void MongoComponent::OnAddTask(RpcTaskComponent<_bson::bsonobj>::RpcTask task)
	{

	}

	std::shared_ptr<_bson::bsonobj> MongoComponent::Run(std::shared_ptr<MongoRequest> request, int flag)
	{
		if (this->mMongoClients.empty())
		{
			return nullptr;
		}
		request->header.responseTo = 0;
		request->header.requestID = this->mRequestId.Pop();
		std::shared_ptr<MongoTask> mongoTask =
				std::make_shared<MongoTask>(request->header.requestID);
		if(!this->AddTask(mongoTask))
		{
			return nullptr;
		}
		int index = flag % this->mMongoClients.size();
		this->mMongoClients[index]->SendMongoCommand(request);
		return mongoTask->Await();
	}

}