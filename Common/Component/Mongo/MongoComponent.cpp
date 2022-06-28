//
// Created by mac on 2022/6/28.
//

#include "MongoComponent.h"

namespace Sentry
{
	MongoTask::MongoTask(int taskId)
		: mTaskId(taskId)
	{

	}

	void MongoTask::OnResponse(std::shared_ptr<Bson::BsonObject> response)
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
		return true;
	}

	bool MongoComponent::OnStart()
	{

	}

	void MongoComponent::OnDelTask(long long taskId, RpcTask task)
	{
		this->mRequestId.Push((int)taskId);
	}

	void MongoComponent::OnAddTask(RpcTaskComponent<Bson::BsonObject>::RpcTask task)
	{

	}

	std::shared_ptr<Bson::BsonObject> MongoComponent::Run(std::shared_ptr<MongoRequest> request, int flag)
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