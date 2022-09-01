//
// Created by mac on 2022/6/28.
//

#include"MongoRpcComponent.h"
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

    std::shared_ptr<TcpMongoClient> MongoRpcComponent::GetClient(int index)
    {
        if(index > 0)
        {
            index = index % this->mMongoClients.size();
            return this->mMongoClients[index];
        }
        std::shared_ptr<TcpMongoClient> returnClient = this->mMongoClients.front();
        for(size_t i = 0; i < this->mMongoClients.size(); i++)
        {
            std::shared_ptr<TcpMongoClient> mongoClient = this->mMongoClients[i];
            if(mongoClient->WaitSendCount() <= 5)
            {
                return mongoClient;
            }
            if(mongoClient->WaitSendCount() < returnClient->WaitSendCount())
            {
                returnClient = mongoClient;
            }
        }
        return returnClient;
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
                Bson::Reader::Document & document = mongoResponse->Get(i);
                if(document.Get("errmsg", json) || document.Get("writeErrors", json))
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

    bool MongoRpcComponent::SetIndex(const std::string &tab, const std::string &name)
    {
        std::shared_ptr<MongoQueryRequest> mongoRequest(new MongoQueryRequest());

        Bson::Writer::Document keys(Bson::DocumentType::Object);
        Bson::Writer::Document document(Bson::DocumentType::Object);
        Bson::Writer::Document documentArray1(Bson::DocumentType::Array);
        keys.Add(name.c_str(), 1);

        document.Add("key", keys);
        document.Add("unique", true);
        document.Add("name", name.c_str());
        mongoRequest->document.Add("createIndexes", tab);
        mongoRequest->document.Add("indexes", documentArray1);
        std::shared_ptr<TcpMongoClient> mongoClient = this->GetClient();
        return this->Run(mongoClient, mongoRequest) != nullptr;
    }

	bool MongoRpcComponent::Ping(int index)
    {
        std::shared_ptr<MongoQueryRequest> mongoRequest
                = std::make_shared<MongoQueryRequest>();
        mongoRequest->document.Add("ping", 1);
        std::shared_ptr<TcpMongoClient> mongoClient = this->GetClient(index);
        return this->Run(mongoClient, mongoRequest) != nullptr;
    }

	void MongoRpcComponent::OnClientError(int index, XCode code)
	{

	}

}