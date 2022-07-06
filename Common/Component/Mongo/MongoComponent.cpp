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
		LOG_CHECK_RET_FALSE(config.GetMember("mongo", "db", this->mConfig.mDb));
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
					std::make_shared<MongoClientContext>(socketProxy, this->mConfig, this, index);
			this->mMongoClients.emplace_back(mongoClientContext);
		}
		Json::Writer jsonDocument1;
		jsonDocument1 << "_id" << 445 << "age" << 10 << "name" << "yjz" << "height" << 175.5f;
		jsonDocument1.BeginArray("arr") << 123 << 234 << 345 << 456 << Json::End::EndArray;
		jsonDocument1.BeginObject("info") << "info1" << 12 << "info2" << "1122" << Json::End::EndObject;

		this->InsertOne("user1", jsonDocument1.JsonString());

		Json::Writer jsonWriter;
		jsonWriter << "_id" << 445;
		this->QueryOnce("user1", jsonWriter.JsonString());

		Json::Writer query;
		query << "_id" << 445;
		Json::Writer update;
		update << "age" << 20 << "name" << "yjz1995..";
		this->Update("user1", update.JsonString(), query.JsonString());

		return this->Ping();
	}

	bool MongoComponent::InsertOne(const std::string& tab, const std::string& json)
	{
		int res = 0;
		Bson::WriterDocument document;
		if(!document.FromByJson(json))
		{
			return false;
		}
		Bson::ArrayDocument documentArray;
		documentArray.Add(document);
		std::shared_ptr<MongoQueryRequest> mongoRequest(new MongoQueryRequest());

		mongoRequest->document.Add("insert", tab);
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

	std::shared_ptr<Bson::ReaderDocument> MongoComponent::Run(std::shared_ptr<MongoQueryRequest> request, int flag)
	{
		if (this->mMongoClients.empty())
		{
			return nullptr;
		}
		request->flag = 0;
		request->numberToSkip = 0;
		request->numberToReturn = 1;
		request->header.responseTo = 0;
		request->header.requestID = this->mRequestId.Pop();
		if(request->collectionName.empty())
		{
			request->collectionName = this->mConfig.mDb + ".$cmd";
		}
		std::shared_ptr<MongoTask> mongoTask = std::make_shared<MongoTask>(request->header.requestID);
		if(!this->AddTask(mongoTask))
		{
			return nullptr;
		}
		long long t1 = Time::GetNowMilTime();
		int index = flag % this->mMongoClients.size();
		this->mMongoClients[index]->PushMongoCommand(request);
		std::shared_ptr<Bson::ReaderDocument> readerDocument = mongoTask->Await();
		if(readerDocument != nullptr)
		{
			std::string json;
			readerDocument->WriterToJson(json);
			LOG_DEBUG( "[" << Time::GetNowMilTime() - t1 << "ms] response = " << json);
			return readerDocument;
		}
		LOG_DEBUG( "[" << Time::GetNowMilTime() - t1 << "ms] response = null");
		return nullptr;
	}

	std::shared_ptr<Bson::ReaderDocument> MongoComponent::QueryOnce(const string& tab, const std::string & json)
	{
		std::shared_ptr<MongoQueryRequest> mongoRequest(new MongoQueryRequest);
		if(!mongoRequest->document.FromByJson(json))
		{
			LOG_ERROR(json << " to bson error");
			return nullptr;
		}
		mongoRequest->collectionName = fmt::format("{0}.{1}", this->mConfig.mDb, tab);
		return this->Run(mongoRequest);
	}

	bool MongoComponent::Update(const std::string& tab, const std::string& update, const std::string& selector)
	{
		Bson::WriterDocument updateDocument;
		if(!updateDocument.FromByJson(update))
		{
			return false;
		}
		Bson::WriterDocument selectorDocument;
		if(!selectorDocument.FromByJson(selector))
		{
			return false;
		}
		std::shared_ptr<MongoQueryRequest> mongoRequest(new MongoQueryRequest);

		Bson::WriterDocument updateInfo;
		updateInfo.Add("upsert", false);
		updateInfo.Add("q", selectorDocument);
		updateInfo.Add("u", updateDocument);

		Bson::ArrayDocument updates;
		updates.Add(updateInfo);
		mongoRequest->document.Add("update", tab);
		mongoRequest->document.Add("updates", updates);
		return this->Run(mongoRequest) != nullptr;
	}

	bool MongoComponent::Ping()
	{
		std::shared_ptr<MongoQueryRequest> mongoRequest(new MongoQueryRequest());
		mongoRequest->document.Add("ping", 1);
		return this->Run(mongoRequest) != nullptr;
	}

	void MongoComponent::OnClientError(int index, XCode code)
	{

	}

}