//
// Created by mac on 2022/6/28.
//

#include "MongoRpcComponent.h"
#include"Component/Scene/NetThreadComponent.h"
namespace Sentry
{
	MongoTask::MongoTask(int id, int ms)
        : IRpcTask<Mongo::MongoQueryResponse>(ms), mTaskId(id) { }

	void MongoTask::OnResponse(std::shared_ptr<Mongo::MongoQueryResponse> response)
	{
		this->mTask.SetResult(response);
	}

    void MongoTask::OnTimeout()
    {
        this->mTask.SetResult(nullptr);
    }
}

namespace Sentry
{
	bool MongoRpcComponent::LateAwake()
	{
		const ServerConfig & config = this->GetApp()->GetConfig();
		this->mTimerComponent = this->GetApp()->GetTimerComponent();
        this->mNetComponent = this->GetComponent<NetThreadComponent>();
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
            const std::string & ip = this->mConfig.mIp;
            const unsigned short port = this->mConfig.mPort;
            std::shared_ptr<SocketProxy> socketProxy = this->mNetComponent->CreateSocket(ip, port);
            std::shared_ptr<TcpMongoClient> mongoClientContext =
                    std::make_shared<TcpMongoClient>(socketProxy, this->mConfig);

            this->mMongoClients.emplace_back(mongoClientContext);
        }
        return this->Ping(0);
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
        std::shared_ptr<TcpMongoClient> mongoClient = this->GetClient();
        std::shared_ptr<Mongo::MongoQueryResponse> response = this->Run(mongoClient, mongoRequest);
		return response != nullptr && response->GetDocumentSize() > 0 && response->Get().IsOk();
	}

    void MongoRpcComponent::SelectMongoClient(int index)
    {
        if(this->mCurClient != nullptr)
        {
            std::move(this->mCurClient);
            this->mCurClient = this->GetClient(index);
        }
    }

    std::shared_ptr<TcpMongoClient> MongoRpcComponent::GetClient(int index)
    {
        if(this->mCurClient != nullptr)
        {
            return std::move(this->mCurClient);
        }
        if(index > 0)
        {
            index = index % this->mMongoClients.size();
            return this->mMongoClients[index];
        }
        std::shared_ptr<TcpMongoClient> retunrClient = this->mMongoClients.front();
        for(size_t i = 0; i < this->mMongoClients.size(); i++)
        {
            std::shared_ptr<TcpMongoClient> mongoClient = this->mMongoClients[i];
            if(mongoClient->WaitSendCount() <= 5)
            {
                return mongoClient;
            }
            if(mongoClient->WaitSendCount() < retunrClient->WaitSendCount())
            {
                retunrClient = mongoClient;
            }
        }
        return retunrClient;
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
        std::shared_ptr<TcpMongoClient> mongoClient = this->GetClient();
		std::shared_ptr<MongoQueryResponse> response = this->Run(mongoClient, mongoRequest);
		return response != nullptr && response->GetDocumentSize() > 0 && response->Get().Get("n", res) && res > 0;
	}

	void MongoRpcComponent::OnDelTask(long long taskId, RpcTask task)
	{
        assert(this->GetApp()->IsMainThread());
        this->mRequestId.Push((int)taskId);
	}

	void MongoRpcComponent::OnAddTask(RpcTaskComponent<Mongo::MongoQueryResponse>::RpcTask task)
	{

	}

	std::shared_ptr<Mongo::MongoQueryResponse> MongoRpcComponent::Run(
            std::shared_ptr<TcpMongoClient> mongoClient, std::shared_ptr<MongoQueryRequest> request)
	{
		if(request->collectionName.empty())
		{
			request->collectionName = this->mConfig.mDb + ".$cmd";
		}
        request->header.requestID = this->mRequestId.Pop();
        std::shared_ptr<MongoTask> mongoTask(new MongoTask(request->header.requestID, 0));
		if(!this->AddTask(mongoTask))
		{
			return nullptr;
		}
		mongoClient->SendMongoCommand(request);
#ifdef __DEBUG__
		long long t1 = Time::GetNowMilTime();
        std::shared_ptr<Mongo::MongoQueryResponse> mongoResponse = mongoTask->Await();
		if(mongoResponse != nullptr && mongoResponse->GetDocumentSize() > 0)
		{
            LOG_DEBUG( "[" << Time::GetNowMilTime() - t1 << "ms] document size = [" << mongoResponse->GetDocumentSize() << "]");
            for(size_t i = 0; i < mongoResponse->GetDocumentSize(); i++)
            {
                std::string json;
                if(mongoResponse->Get(i).Get("errmsg", json))
                {
                    LOG_ERROR("error[" << i << "] = " << json);
                }
                else
                {
                    mongoResponse->Get(i).WriterToJson(json);
                    CONSOLE_LOG_DEBUG("json[" << i << "] = " << json);
                }
            }
            return mongoResponse;
		}

		CONSOLE_LOG_DEBUG(request->collectionName << "[" << Time::GetNowMilTime() - t1 << "ms] response = null");
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
        std::shared_ptr<TcpMongoClient> mongoClient = this->GetClient();
        mongoRequest->collectionName = fmt::format("{0}.{1}", this->mConfig.mDb, tab);
		return this->Run(mongoClient, mongoRequest);
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
        std::shared_ptr<TcpMongoClient> mongoClient = this->GetClient();
        return this->Run(mongoClient, mongoRequest) != nullptr;
	}

    bool MongoRpcComponent::SetIndex(const std::string &tab, const std::string &name)
    {
        std::shared_ptr<MongoQueryRequest> mongoRequest(new MongoQueryRequest());

        Bson::Writer::Object keys;
        keys.Add(name.c_str(), 1);

        Bson::Writer::Object document;
        document.Add("key", keys);
        document.Add("unique", true);
        document.Add("name", name.c_str());
        Bson::Writer::Array documentArray1(document);
        mongoRequest->document.Add("createIndexes", tab);
        mongoRequest->document.Add("indexes", documentArray1);
        std::shared_ptr<TcpMongoClient> mongoClient = this->GetClient();
        return this->Run(mongoClient, mongoRequest) != nullptr;
    }

	bool MongoRpcComponent::Ping(int index)
	{
        std::shared_ptr<TcpMongoClient> mongoClient = this->GetClient(index);
        std::shared_ptr<MongoQueryRequest> mongoRequest(new MongoQueryRequest());
		mongoRequest->document.Add("ismaster", 1);
		return this->Run(mongoClient, mongoRequest) != nullptr;
	}

	void MongoRpcComponent::OnClientError(int index, XCode code)
	{

	}

}