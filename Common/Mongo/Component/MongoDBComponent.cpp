//
// Created by mac on 2022/6/28.
//

#include"MongoDBComponent.h"
#include"Server/Config/ServerConfig.h"
#include"Server/Component/ThreadComponent.h"
#include "Util/Time/TimeHelper.h"
#include"Mongo/Lua/LuaMongo.h"
#include"Lua/Engine/ClassProxyHelper.h"
namespace Tendo
{
	MongoTask::MongoTask(int id)
        : IRpcTask<Mongo::CommandResponse>(id) { }

	void MongoTask::OnResponse(std::shared_ptr<Mongo::CommandResponse> response)
	{
		this->mTask.SetResult(response);
	}
}

namespace Tendo
{
    MongoDBComponent::MongoDBComponent()
    {
        this->mIndex = 0;
        this->mWaitCount = 0;     
    }

    int MongoDBComponent::MakeMongoClient(const Mongo::MongoConfig& config)
    {
        const std::string& ip = config.Address.Ip;
        const unsigned short port = config.Address.Port;
        ThreadComponent* threadComponent = this->GetComponent<ThreadComponent>();
        {
            this->mIndex++;
            std::shared_ptr<Tcp::SocketProxy> socketProxy = threadComponent->CreateSocket(ip, port);
            std::shared_ptr<Mongo::TcpMongoClient> mongoClientContext = std::make_shared<Mongo::TcpMongoClient>(socketProxy, this, config);
            this->mMongoClients.emplace(this->mIndex, mongoClientContext);
        }
        return this->mIndex;
    }

    
    void MongoDBComponent::OnDestroy()
    {
        this->mIndex = 0;
        auto iter = this->mMongoClients.begin();
        for (; iter != this->mMongoClients.end(); ++iter)
        {
            iter->second->Stop();
        }
        this->mMongoClients.clear();
    }


	void MongoDBComponent::OnConnectSuccessful(const std::string& address)
	{
		LOG_INFO("mongo client [" << address << "] auth successful");
	}

	void MongoDBComponent::OnMessage(std::shared_ptr<Mongo::CommandResponse> message)
	{
		this->OnResponse(message->GetHead().responseTo, message);
	}

    Mongo::TcpMongoClient * MongoDBComponent::GetClient(int index)
    {       
        if (this->mMongoClients.empty())
        {
            return nullptr;
        }
        auto iter = this->mMongoClients.find(index);
        if (iter != this->mMongoClients.end())
        {
            return iter->second.get();
        }
        return this->mMongoClients.begin()->second.get();
    }

    void MongoDBComponent::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
    {
        luaRegister.BeginNewTable("Mongo");
        luaRegister.PushExtensionFunction("Make", Lua::LuaMongo::Make);
        luaRegister.PushExtensionFunction("Exec", Lua::LuaMongo::Exec);
        luaRegister.PushExtensionFunction("Query", Lua::LuaMongo::Query);
        luaRegister.PushExtensionFunction("QueryOnce", Lua::LuaMongo::QueryOnce);
    }

    bool MongoDBComponent::Send(int id, const std::shared_ptr<Mongo::CommandRequest>& request, int& taskId)
    {
        Mongo::TcpMongoClient* mongoClient = this->GetClient(id);
        if (mongoClient == nullptr)
        {
            return false;
        }
        this->mWaitCount++;
        if(request->collectionName.empty())
        {
            request->collectionName = request->dataBase + ".$cmd";
        }
        taskId = this->PopTaskId();
        request->header.requestID = taskId;
        mongoClient->SendMongoCommand(request);
        return true;
    }

	std::shared_ptr<Mongo::CommandResponse> MongoDBComponent::Run(
        int id, const std::shared_ptr<Mongo::CommandRequest>& request)
	{      
        int taskId = 0;
        if (!this->Send(id, request, taskId))
        {
            return nullptr;
        }   
        std::shared_ptr<MongoTask> mongoTask(new MongoTask(taskId));	
#ifdef __DEBUG__
		long long t1 = Helper::Time::NowMilTime();
        std::shared_ptr<Mongo::CommandResponse> mongoResponse = this->AddTask(taskId, mongoTask)->Await();
        {
            this->mWaitCount--;
        }
		if(mongoResponse != nullptr && mongoResponse->GetDocumentSize() > 0)
		{
            //LOG_DEBUG( "[" << Time::NowMilTime() - t1 << "ms] document size = [" << mongoResponse->GetDocumentSize() << "]");
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

		CONSOLE_LOG_DEBUG(request->collectionName << "[" << Helper::Time::NowMilTime() - t1 << "ms] response = null");
		return nullptr;
#else
        return  this->AddTask(taskId, mongoTask)->Await();
#endif
	}

    bool MongoDBComponent::SetIndex(int id, const std::string &tab, const std::string &name)
    {
        const std::shared_ptr<Mongo::CommandRequest> mongoRequest = std::make_shared<Mongo::CommandRequest>();
        {
            Bson::Writer::Document keys;
            Bson::Writer::Document document;
            keys.Add(name.c_str(), 1);

            document.Add("key", keys);
            document.Add("unique", true); //是否为唯一索引
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
        }
        return this->Run(0, mongoRequest) != nullptr;
    }

	bool MongoDBComponent::Ping(int id)
    {
        std::shared_ptr<Mongo::CommandRequest> mongoRequest = std::make_shared<Mongo::CommandRequest>();
        {
            mongoRequest->dataBase = "admin";
            mongoRequest->document.Add("ping", 1);
        }
        return this->Run(id, mongoRequest) != nullptr;
    }

	void MongoDBComponent::OnClientError(int index, int code)
	{

	}

}