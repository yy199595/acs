//
// Created by mac on 2022/6/28.
//

#include"MongoDBComponent.h"
#include"Config/MongoConfig.h"
#include"Component/NetThreadComponent.h"
namespace Sentry
{
	MongoTask::MongoTask(int id, int ms)
        : IRpcTask<Mongo::CommandResponse>(ms), mTaskId(id) { }

	void MongoTask::OnResponse(std::shared_ptr<Mongo::CommandResponse> response)
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
    void MongoDBComponent::CloseClients()
    {
        for(std::shared_ptr<TcpMongoClient> client : this->mMongoClients)
        {
            client->Stop();
        }
        this->mMongoClients.clear();
    }

	bool MongoDBComponent::Start()
    {
        LOG_CHECK_RET_FALSE(MongoConfig::Inst());
        const MongoConfig * config = MongoConfig::Inst();
        NetThreadComponent * threadComponent = this->GetComponent<NetThreadComponent>();
        for (int index = 0; index < config->MaxCount; index++)
        {
            const std::string & ip = config->Ip;
            const unsigned short port = config->Port;
            std::shared_ptr<SocketProxy> socketProxy = threadComponent->CreateSocket(ip, port);
            std::shared_ptr<TcpMongoClient> mongoClientContext = std::make_shared<TcpMongoClient>(socketProxy, this);

            this->mMongoClients.emplace_back(mongoClientContext);
        }
        return this->Ping(0);
    }

	void MongoDBComponent::OnConnectSuccessful(const std::string& address)
	{
		LOG_INFO("mongo client [" << address << "] auth successful");
	}

	void MongoDBComponent::OnMessage(const std::string& address, std::shared_ptr<CommandResponse> message)
	{
		int taskId = message->GetHead().responseTo;
		this->OnResponse(taskId, message);
	}

    TcpMongoClient * MongoDBComponent::GetClient(int index)
    {
        if(index > 0)
        {
            index = index % this->mMongoClients.size();
            return this->mMongoClients[index].get();
        }
        std::shared_ptr<TcpMongoClient> returnClient = this->mMongoClients.front();
        for(size_t i = 0; i < this->mMongoClients.size(); i++)
        {
            std::shared_ptr<TcpMongoClient> mongoClient = this->mMongoClients[i];
            if(mongoClient->WaitSendCount() <= 5)
            {
                return mongoClient.get();
            }
            if(mongoClient->WaitSendCount() < returnClient->WaitSendCount())
            {
                returnClient = mongoClient;
            }
        }
        return returnClient.get();
    }

	void MongoDBComponent::OnDelTask(int taskId, RpcTask task)
	{
        assert(this->mApp->IsMainThread());
        this->mRequestId.Push(taskId);
	}

	void MongoDBComponent::OnAddTask(RpcTaskComponent<int, Mongo::CommandResponse>::RpcTask task)
	{

	}

    void MongoDBComponent::Send(TcpMongoClient * mongoClient, std::shared_ptr<CommandRequest> request)
    {

    }

	std::shared_ptr<Mongo::CommandResponse> MongoDBComponent::Run(
        TcpMongoClient * mongoClient, std::shared_ptr<CommandRequest> request)
	{
		if(request->collectionName.empty())
		{
			request->collectionName = request->dataBase + ".$cmd";
		}
        request->header.requestID = this->mRequestId.Pop();

        mongoClient->SendMongoCommand(request);
        std::shared_ptr<MongoTask> mongoTask(new MongoTask(request->header.requestID, 0));

#ifdef __DEBUG__
		long long t1 = Time::GetNowMilTime();
        int taskId = (int)mongoTask->GetRpcId();
        std::shared_ptr<Mongo::CommandResponse> mongoResponse = this->AddTask(taskId, mongoTask)->Await();
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
        return  this->AddTask(mongoTask)->Await();
#endif
	}

    bool MongoDBComponent::SetIndex(const std::string &tab, const std::string &name)
    {
        std::shared_ptr<CommandRequest> mongoRequest
            = std::make_shared<CommandRequest>();

        Bson::Writer::Document keys;
        Bson::Writer::Document document;
        keys.Add(name.c_str(), 1);

        document.Add("key", keys);
        document.Add("unique", true);
        document.Add("name", name.c_str());
        Bson::Writer::Array documentArray1(document);

        const size_t pos = tab.find('.');
        if(pos == std::string::npos)
        {
            return false;
        }

        std::string tab1 = tab.substr(pos + 1);
        mongoRequest->dataBase = tab.substr(0, pos);
        mongoRequest->document.Add("createIndexes", tab1);
        mongoRequest->document.Add("indexes", documentArray1);
        return this->Run(this->GetClient(), mongoRequest) != nullptr;
    }

	bool MongoDBComponent::Ping(int index)
    {
        std::shared_ptr<CommandRequest> mongoRequest
                = std::make_shared<CommandRequest>();

        mongoRequest->dataBase = MongoConfig::Inst()->DB;
        mongoRequest->document.Add("ping", 1);
        return this->Run(this->GetClient(index), mongoRequest) != nullptr;
    }

	void MongoDBComponent::OnClientError(int index, XCode code)
	{

	}

}