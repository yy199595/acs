//
// Created by mac on 2022/6/28.
//

#include "MongoRpcComponent.h"
#include"Component/Scene/NetThreadComponent.h"
namespace Sentry
{
	MongoTask::MongoTask(int id)
        : mTaskId(id)
	{

	}

	void MongoTask::OnResponse(std::shared_ptr<Mongo::MongoQueryResponse> response)
	{
		this->mTask.SetResult(response);
	}
}

namespace Sentry
{
	bool MongoRpcComponent::LateAwake()
	{
        this->mIndex = 0;
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

	bool MongoRpcComponent::OnStart()
    {
        for (int index = 0; index < this->mConfig.mMaxCount; index++)
        {
#ifdef ONLY_MAIN_THREAD
            IAsioThread & asioThread = this->GetApp()->GetTaskScheduler();
#else
            NetThreadComponent *netThreadComponent = this->GetComponent<NetThreadComponent>();
            IAsioThread &asioThread = netThreadComponent->AllocateNetThread();
#endif
            std::shared_ptr<SocketProxy> socketProxy =
                    std::make_shared<SocketProxy>(asioThread, this->mConfig.mIp, this->mConfig.mPort);
            std::shared_ptr<MongoClientContext> mongoClientContext =
                    std::make_shared<MongoClientContext>(socketProxy, this->mConfig, this, index);

            this->mMongoClients.emplace_back(mongoClientContext);
            if (!this->Ping(index))
            {
                return false;
            }
        }
//        for (int index = 0; index < 10; index++)
//        {
//            this->GetApp()->GetTaskComponent()->Start([this]() {
//                Json::Writer json;
//                json << "_id" << fmt::format("{0}@qq.com", 646585122);
//                while (true)
//                {
//                    std::string str = json.JsonString();
//                    this->Query("account_user", str, 1);
//                }
//            });
//        }

        return true;
    }

	bool MongoRpcComponent::Delete(const std::string& tab, const std::string& json, int limit)
	{
		Bson::Writer::Object document;
		if(!document.FromByJson(json))
		{
			return false;
		}
		Bson::Writer::Object delDocument;
		delDocument.Add("q", document);
		delDocument.Add("limit", limit);

		Bson::Writer::Array documentArray(delDocument);
		std::shared_ptr<MongoQueryRequest> mongoRequest(new MongoQueryRequest());

		mongoRequest->document.Add("delete", tab);
		mongoRequest->document.Add("deletes", documentArray);
		std::shared_ptr<Mongo::MongoQueryResponse> response = this->Run(mongoRequest);
		return response != nullptr && response->GetDocumentSize() > 0 && response->Get().IsOk();
	}

	bool MongoRpcComponent::InsertOnce(const std::string& tab, const std::string& json)
	{
		int res = 0;
		Bson::Writer::Object document;
		if(!document.FromByJson(json))
		{
			return false;
		}
		Bson::Writer::Array documentArray(document);
		std::shared_ptr<MongoQueryRequest> mongoRequest(new MongoQueryRequest());

		mongoRequest->document.Add("insert", tab);
		mongoRequest->document.Add("documents", documentArray);
		std::shared_ptr<MongoQueryResponse> response = this->Run(mongoRequest);
		return response != nullptr && response->GetDocumentSize() > 0 && response->Get().Get("n", res) && res > 0;
	}

	void MongoRpcComponent::OnDelTask(long long taskId, RpcTask task)
	{
		this->mRequestId.Push((int)taskId);
	}

	void MongoRpcComponent::OnAddTask(RpcTaskComponent<Mongo::MongoQueryResponse>::RpcTask task)
	{

	}

	std::shared_ptr<Mongo::MongoQueryResponse> MongoRpcComponent::Run(std::shared_ptr<MongoQueryRequest> request, int flag)
	{
		if (this->mMongoClients.empty())
		{
			return nullptr;
		}
		request->header.requestID = 1;
		if(request->collectionName.empty())
		{
			request->collectionName = this->mConfig.mDb + ".$cmd";
		}
        request->mTaskId = this->mRequestId.Pop();
		std::shared_ptr<MongoTask> mongoTask(new MongoTask(request->mTaskId));
		if(!this->AddTask(mongoTask))
		{
			return nullptr;
		}
        int index = this->mIndex % this->mMongoClients.size();
		this->mMongoClients[index]->SendMongoCommand(request);
        this->mIndex++;
#ifdef __DEBUG__
		assert(this->IsMainThread());
		long long t1 = Time::GetNowMilTime();
        std::shared_ptr<Mongo::MongoQueryResponse> mongoResponse = mongoTask->Await();
		if(mongoResponse != nullptr && mongoResponse->GetDocumentSize() > 0)
		{
            LOG_DEBUG( "[" << Time::GetNowMilTime() - t1 << "ms] document size = [" << mongoResponse->GetDocumentSize() << "]");
            for(size_t i = 0; i < mongoResponse->GetDocumentSize(); i++)
            {
                std::string json;
                mongoResponse->Get(i).WriterToJson(json);
                LOG_DEBUG("[" << i << "] " << json);
            }
            return mongoResponse;
		}

		LOG_DEBUG( "[" << Time::GetNowMilTime() - t1 << "ms] response = null");
		return nullptr;
#else
        return  mongoTask->Await();
#endif
	}

     // $gt:大于   $lt:小于  $gte:大于或等于  $lte:小于或等于 $ne:不等于
	std::shared_ptr<MongoQueryResponse> MongoRpcComponent::Query(const string& tab, const std::string & json, int limit)
	{
		std::shared_ptr<MongoQueryRequest> mongoRequest(new MongoQueryRequest);
		if(!mongoRequest->document.FromByJson(json))
		{
			LOG_ERROR(json << " to bson error");
			return nullptr;
		}
        mongoRequest->numberToReturn = limit;
		mongoRequest->collectionName = fmt::format("{0}.{1}", this->mConfig.mDb, tab);
		return this->Run(mongoRequest);
	}

	bool MongoRpcComponent::Update(const std::string& tab, const std::string& update, const std::string& selector, const std::string & tag)
	{
		Bson::Writer::Object dataDocument;
		if(!dataDocument.FromByJson(update))
		{
			return false;
		}
		Bson::Writer::Object selectorDocument;
		if(!selectorDocument.FromByJson(selector))
		{
			return false;
		}
		Bson::Writer::Object updateDocument;
		updateDocument.Add(tag.c_str(), dataDocument);
		std::shared_ptr<MongoQueryRequest> mongoRequest(new MongoQueryRequest);

		Bson::Writer::Object updateInfo;
		updateInfo.Add("multi", true);
		updateInfo.Add("upsert", false);
		updateInfo.Add("q", selectorDocument);
		updateInfo.Add("u", updateDocument);

		Bson::Writer::Array updates;
		updates.Add(updateInfo);
		mongoRequest->document.Add("update", tab);
		mongoRequest->document.Add("updates", updates);
		return this->Run(mongoRequest) != nullptr;
	}

	bool MongoRpcComponent::Ping(int index)
	{
		std::shared_ptr<MongoQueryRequest> mongoRequest(new MongoQueryRequest());
		mongoRequest->document.Add("ping", 1);
		return this->Run(mongoRequest, index) != nullptr;
	}

	void MongoRpcComponent::OnClientError(int index, XCode code)
	{

	}

}