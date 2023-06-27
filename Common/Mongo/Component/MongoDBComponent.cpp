//
// Created by mac on 2022/6/28.
//

#include"MongoDBComponent.h"
#include"Entity/Actor/App.h"
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

    int MongoDBComponent::MakeMongoClient(const Mongo::MongoConfig& config, bool async)
    {
		this->mIndex++;
		const std::string& ip = config.Address.Ip;
        const unsigned short port = config.Address.Port;
		if(async)
		{
			ThreadComponent* threadComponent = this->GetComponent<ThreadComponent>();
			{
				std::shared_ptr<Tcp::SocketProxy> socketProxy = threadComponent->CreateSocket(ip, port);
				std::shared_ptr<Mongo::TcpMongoClient> mongoClientContext = std::make_shared<Mongo::TcpMongoClient>(socketProxy, this, config);
				this->mMongoClients.emplace(this->mIndex, mongoClientContext);
			}
		}
		else
		{
			Asio::Context & io = this->mApp->MainThread();
			std::shared_ptr<Tcp::SocketProxy> socketProxy = std::make_shared<Tcp::SocketProxy>(io);
			{
				socketProxy->Init(ip, port);
				std::shared_ptr<Mongo::TcpMongoClient> mongoClientContext =
						std::make_shared<Mongo::TcpMongoClient>(socketProxy, config);
				LOG_CHECK_RET_ZERO(mongoClientContext->SyncAuth());
				this->mSyncMongoClients.emplace(this->mIndex, mongoClientContext);
			}
		}
        return this->mIndex;
    }

	void MongoDBComponent::Start()
	{
		this->Ping();
	}

	void MongoDBComponent::OnSecondUpdate(int tick)
	{
		if(tick % 60 != 0)
		{
			return;
		}
		//this->mApp->GetCoroutine()->Start(&MongoDBComponent::Ping, this);
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
		int rpc = message->RpcId();
		this->OnResponse(rpc, message);
	}

	std::shared_ptr<Mongo::CommandResponse> MongoDBComponent::Run(
        int id, const std::shared_ptr<Mongo::CommandRequest>& request)
	{
#ifdef __DEBUG__
		long long t1 = Helper::Time::NowMilTime();
#endif
		std::shared_ptr<Mongo::CommandResponse> mongoResponse = this->AsyncRun(id, request);
		if(mongoResponse == nullptr)
		{
			mongoResponse = this->SyncRun(id, request);
			if(mongoResponse == nullptr)
			{
				this->mWaitCount--;
				return mongoResponse;
			}
		}

#ifdef __DEBUG__
		Json::Writer jsonWriter(false);
		//CONSOLE_LOG_DEBUG("mongo request = " << request->document.ToJson());
		if(mongoResponse != nullptr && mongoResponse->Document()  != nullptr)
		{
			Bson::Reader::Document * document1 = mongoResponse->Document();
			{
				std::string json;
				rapidjson::Document document;
				document1->WriterToJson(&json);
				if (!document.Parse(json.c_str(), json.size()).HasParseError())
				{
					jsonWriter.Add(document);
					if ((document.HasMember("errmsg") || document.HasMember("writeErrors")))
					{
						LOG_ERROR("mongo response = " << jsonWriter.JsonString());
						return mongoResponse;
					}
				}
			}
			//CONSOLE_LOG_DEBUG("mongo response = " << jsonWriter.JsonString());
            return mongoResponse;
		}

		CONSOLE_LOG_DEBUG(request->collectionName << " [" << Helper::Time::NowMilTime() - t1 << "ms] response = null");
		return mongoResponse;
#else
        return mongoResponse;
#endif
	}

	std::shared_ptr<Mongo::CommandResponse> MongoDBComponent::SyncRun(int id, const std::shared_ptr<Mongo::CommandRequest>& request)
	{
		Mongo::TcpMongoClient* mongoClient = nullptr;
		auto iter = this->mSyncMongoClients.find(id);
		if(iter == this->mSyncMongoClients.end())
		{
			return nullptr;
		}
		this->mWaitCount++;
		mongoClient = iter->second.get();
		if(request->collectionName.empty())
		{
			request->collectionName = request->dataBase + ".$cmd";
		}
		request->header.requestID = this->PopTaskId();
		return mongoClient->SyncSendMongoCommand(request);
	}

	std::shared_ptr<Mongo::CommandResponse> MongoDBComponent::AsyncRun(int id, const std::shared_ptr<Mongo::CommandRequest>& request)
	{
		Mongo::TcpMongoClient* mongoClient = nullptr;
		auto iter = this->mMongoClients.find(id);
		if(iter == this->mMongoClients.end())
		{
			return nullptr;
		}
		this->mWaitCount++;
		mongoClient = iter->second.get();
		if(request->collectionName.empty())
		{
			request->collectionName = request->dataBase + ".$cmd";
		}
		int taskId = this->PopTaskId();
		request->header.requestID = taskId;
		mongoClient->SendMongoCommand(request);
		std::shared_ptr<MongoTask> mongoTask = std::make_shared<MongoTask>(taskId);
		{
			this->AddTask(taskId, mongoTask);
		}
		return mongoTask->Await();
	}

	void MongoDBComponent::Ping()
	{
		auto iter = this->mMongoClients.begin();
		long long t1 = Helper::Time::NowMilTime();
		for (; iter != this->mMongoClients.end(); iter++)
		{
			int count = 0;
			int id = iter->first;
			std::shared_ptr<Mongo::CommandRequest> mongoRequest = iter->second->MakePing();
			while (this->Run(id, mongoRequest) == nullptr)
			{
				count++;
				LOG_DEBUG("ping mongo server : ["
						<< iter->second->GetAddress() << "] count = " << count);
				this->mApp->GetCoroutine()->Sleep(5000);
			}
		}
		long long t2 = Helper::Time::NowMilTime();
		LOG_INFO("ping mongo server time = [" << t2 - t1 << "ms]");
	}
}