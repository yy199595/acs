//
// Created by mac on 2022/6/28.
//

#include"MongoDBComponent.h"
#include"Server/Config/ServerConfig.h"
#include"Server/Component/ThreadComponent.h"
#include "Util/Time/TimeHelper.h"
#include"Mongo/Lua/LuaMongo.h"
#include"Lua/Engine/ClassProxyHelper.h"
#ifdef __DEBUG__
#include"Util/String/StringHelper.h"
#endif
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
        this->mWaitCount = 0;     
    }

	bool MongoDBComponent::Awake()
	{
		std::string path;
		const ServerConfig * config = ServerConfig::Inst();
		LOG_CHECK_RET_FALSE(config->GetPath("db", path));
		return this->mConfig.LoadConfig(path);
	}

    bool MongoDBComponent::LateAwake()
    {
        int id = 0;
        for (int index = 0; index < this->mConfig.MaxCount; index++)
        {
            id++;
            const std::string& ip = this->mConfig.Address[0].Ip;
            const unsigned int port = this->mConfig.Address[0].Port;
            ThreadComponent* threadComponent =
                this->GetComponent<ThreadComponent>();

            std::shared_ptr<Tcp::SocketProxy> socketProxy =
                threadComponent->CreateSocket(ip, port);

            std::shared_ptr<TcpMongoClient> mongoClientContext =
                std::make_shared<TcpMongoClient>(socketProxy, this, this->mConfig);

            this->mAllotQueue.push(id);
            this->mMongoClients.emplace(id, mongoClientContext);
        }
        return true;
    }

    void MongoDBComponent::OnDestroy()
    {
        while (!this->mAllotQueue.empty())
        {
            this->mAllotQueue.pop();
        }
        auto iter = this->mMongoClients.begin();
        for (; iter != this->mMongoClients.end(); iter++)
        {
            iter->second->Stop();
        }
        this->mMongoClients.clear();
    }


	void MongoDBComponent::OnConnectSuccessful(const std::string& address)
	{
		LOG_INFO("mongo client [" << address << "] auth successful");
	}

	void MongoDBComponent::OnMessage(std::shared_ptr<CommandResponse> message)
	{
		int taskId = message->GetHead().responseTo;
		this->OnResponse(taskId, message);
	}

    TcpMongoClient * MongoDBComponent::GetClient(int index)
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

    bool MongoDBComponent::GetClientHandler(int& id)
    {
        if (this->mAllotQueue.empty())
        {
            return false;
        }
        id = this->mAllotQueue.front();
        this->mAllotQueue.pop();
        this->mAllotQueue.push(id);
        return true;
    }

    bool MongoDBComponent::Send(int id, std::shared_ptr<CommandRequest> request, int& taskId)
    {
        TcpMongoClient* mongoClient = this->GetClient(id);
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
        int id, const std::shared_ptr<CommandRequest>& request)
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
        return this->Run(0, mongoRequest) != nullptr;
    }

	bool MongoDBComponent::Ping(int id)
    {
        std::shared_ptr<CommandRequest> mongoRequest
                = std::make_shared<CommandRequest>();

        mongoRequest->dataBase = this->mConfig.DB;
        mongoRequest->document.Add("ping", 1);
        return this->Run(id, mongoRequest) != nullptr;
    }

	void MongoDBComponent::OnClientError(int index, int code)
	{

	}

}