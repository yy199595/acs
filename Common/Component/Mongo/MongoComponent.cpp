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

	void MongoTask::OnResponse(std::shared_ptr<Bson::ReaderDocument> response)
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
		for (int index = 0; index < this->mConfig.mMaxCount; index++)
		{
#ifdef ONLY_MAIN_THREAD
			IAsioThread & asioThread = this->GetApp()->GetTaskScheduler();
#else
			NetThreadComponent* netThreadComponent = this->GetComponent<NetThreadComponent>();
			IAsioThread& asioThread = netThreadComponent->AllocateNetThread();
#endif
			std::shared_ptr<SocketProxy> socketProxy =
					std::make_shared<SocketProxy>(asioThread, this->mConfig.mIp, this->mConfig.mPort);
			std::shared_ptr<MongoClientContext> mongoClientContext =
					std::make_shared<MongoClientContext>(socketProxy, this->mConfig, this);
			this->mMongoClients.emplace_back(mongoClientContext);
		}
// bson 测试开始
		Bson::WriterDocument document;
		document.Add("name", "yjz");
		document.Add("age", 27);
		document.Add("lenght", 170.5f);

		Bson::ArrayDocument document1;
		for(int index = 0; index < 10; index++)
		{
			document1.Add(index * 100);
		}

		Bson::WriterDocument document2;
		document2.Add("11223", 33445);
		document2.Add("22334", 66778);

		std::string str;
		document.Add("arr", document1);
		document.Add("obj", document2);

		int len;
		const char * bson = document.Serialize(len);

		Bson::ReaderDocument readerDocument(bson);
		readerDocument.WriterToJson(str);
// bson测试结束

		std::shared_ptr<MongoQueryRequest> mongoRequest(new MongoQueryRequest());

		bool res = false;
		mongoRequest->flag = 0;
		mongoRequest->numberToSkip = 0;
		mongoRequest->numberToReturn = 1;
		mongoRequest->collectionName = "admin.$cmd";
		mongoRequest->document.Add("ismaster", 1);
		std::shared_ptr<Bson::ReaderDocument> mongoResponse = this->Run(mongoRequest);

		std::string json;
		mongoResponse->WriterToJson(json);
		return mongoResponse != nullptr && mongoResponse->Get("ismaster", res) && res;
	}

	void MongoComponent::OnDelTask(long long taskId, RpcTask task)
	{
		this->mRequestId.Push((int)taskId);
	}

	void MongoComponent::OnAddTask(RpcTaskComponent<Bson::ReaderDocument>::RpcTask task)
	{

	}

	std::shared_ptr<Bson::ReaderDocument> MongoComponent::Run(std::shared_ptr<MongoRequest> request, int flag)
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