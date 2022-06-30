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
		std::string str;
		Bson::WriterDocument document;
		//document.Add("_id", 444);
		std::shared_ptr<Bson::ReaderDocument> readerDocument = this->QueryOnce("ET", "UserData", document);
		readerDocument->WriterToJson(str);
		return true;
	}

	bool MongoComponent::InsertOne(const std::string& db, const std::string& tab, Bson::WriterDocument& document)
	{
		int res = 0;
		Bson::ArrayDocument documentArray;
		documentArray.Add(document);
		std::shared_ptr<MongoQueryRequest> mongoRequest
				= this->GetRequest(db, tab, "insert");
		mongoRequest->document.Add("documents", documentArray);
		std::shared_ptr<Bson::ReaderDocument> response = this->Run(mongoRequest);
		return response != nullptr && response->Get("n", res) && res > 0;
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
	std::shared_ptr<MongoQueryRequest> MongoComponent::GetRequest(
		const std::string& db, const std::string& tab, const std::string & cmd)
	{
		std::shared_ptr<MongoQueryRequest> mongoRequest(new MongoQueryRequest());

		mongoRequest->flag = 0;
		mongoRequest->numberToSkip = 0;
		mongoRequest->numberToReturn = 1;
		mongoRequest->collectionName = db + ".$cmd";
		mongoRequest->document.Add(cmd.c_str(), tab);
		return mongoRequest;
	}
	std::shared_ptr<Bson::ReaderDocument> MongoComponent::QueryOnce(
		const string& db, const string& tab, Bson::WriterDocument& document)
	{
		std::shared_ptr<MongoQueryRequest> mongoRequest
			= this->GetRequest(db, tab, "find");
		mongoRequest->document.Add("query", document);
		return this->Run(mongoRequest);
	}

}