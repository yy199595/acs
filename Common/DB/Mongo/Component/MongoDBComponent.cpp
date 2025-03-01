//
// Created by mac on 2022/6/28.
//
#include"MongoDBComponent.h"
#include"Entity/Actor/App.h"
#include"Util/Tools/TimeHelper.h"
#include"Yyjson/Lua/ljson.h"
#include"Lua/Engine/ModuleClass.h"
#include"Timer/Timer/ElapsedTimer.h"
#include"Server/Config/ServerConfig.h"
#include"Server/Component/ThreadComponent.h"
#include "Proto/Component/ProtoComponent.h"
// use admin 	=>db.createUser({ user: 'root', pwd: '199595yjz.', roles: [{ role: 'root', db: 'yjz' }] })
// use yjz 		=> db.createUser({ user: 'root', pwd: '199595yjz.', roles: [{ role: 'readWrite', db: 'yjz' }] })

const std::string LOG_NAME("mongo");

namespace acs
{
	MongoTask::MongoTask(int id)
        : IRpcTask<mongo::Response>(id), mMessage(nullptr) { }

	LuaMongoTask::LuaMongoTask(lua_State* lua, int id)
		: mLua(lua), IRpcTask<mongo::Response>(id)
	{
		this->mRef = 0;
		if(lua_isthread(this->mLua, -1))
		{
			this->mRef = luaL_ref(lua, LUA_REGISTRYINDEX);
		}
	}

	LuaMongoTask::~LuaMongoTask()
	{
		if(this->mRef != 0)
		{
			luaL_unref(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		}
	}

	int LuaMongoTask::Await() noexcept
	{
		if(this->mRef == 0)
		{
			luaL_error(this->mLua, "not lua coroutine context yield failure");
			return 0;
		}
		return lua_yield(this->mLua, 0);
	}

	void LuaMongoTask::OnResponse(std::unique_ptr<mongo::Response> response) noexcept
	{
		if (response != nullptr && response->Document())
		{
			std::string json;
			if(response->Document()->WriterToJson(&json))
			{
				lua::yyjson::write(this->mLua, json.c_str(), json.size());
			}
		}
		else
		{
			lua_pushnil(this->mLua);
		}
		Lua::Coroutine::Resume(this->mLua, 1);
	}
}


namespace acs
{
	MongoDBComponent::MongoDBComponent()
	{
		mongo::MongoConfig::RegisterField("db", &mongo::MongoConfig::DB);
		mongo::MongoConfig::RegisterField("ping", &mongo::MongoConfig::Ping);
		mongo::MongoConfig::RegisterField("user", &mongo::MongoConfig::User);
		mongo::MongoConfig::RegisterField("debug", &mongo::MongoConfig::Debug);
		mongo::MongoConfig::RegisterField("log", &mongo::MongoConfig::LogPath);
		mongo::MongoConfig::RegisterField("count", &mongo::MongoConfig::MaxCount);
		mongo::MongoConfig::RegisterField("passwd", &mongo::MongoConfig::Password);
		mongo::MongoConfig::RegisterField("address", &mongo::MongoConfig::Address, true);
	}

	bool MongoDBComponent::Awake()
	{
		ServerConfig & config = this->mApp->GetConfig();
		LOG_CHECK_RET_FALSE(config.Get("mongo", this->mConfig));
		this->mCommand = fmt::format("{}.$cmd", this->mConfig.DB);
		return true;
	}

	bool MongoDBComponent::LateAwake()
	{
		ThreadComponent* threadComponent = this->GetComponent<ThreadComponent>();
		for (int index = 0; index < this->mConfig.MaxCount && threadComponent; index++)
		{
			mongo::MongoConfig config = this->mConfig;
			{
				config.Index = index + 1;
				timer::ElapsedTimer timer1;
				Asio::Context & io = this->mApp->GetContext();
				tcp::Socket * socketProxy = threadComponent->CreateSocket(this->mConfig.Address);
				std::shared_ptr<mongo::Client> mongoClientContext = std::make_unique<mongo::Client>(socketProxy, this, config, io);
				{
					if(!mongoClientContext->Start())
					{
						const std::string & address = config.Address;
						LOG_ERROR("connect mongo [{}] fail index = {}", address, config.Index);
						return false;
					}
				}
				this->mFreeClients.Push(config.Index);
				const std::string & address = this->mConfig.Address;
				this->mMongoClients.emplace(config.Index, mongoClientContext);
				LOG_INFO("[{}ms] connect mongo [{}] ok", timer1.GetMs(), address);
			}
		}
		return true;
	}
    
    void MongoDBComponent::OnDestroy()
    {
		auto iter = this->mMongoClients.begin();
		for(; iter != this->mMongoClients.end(); iter++)
		{
			iter->second->Stop();
		}
		this->mMongoClients.clear();
    }

	void MongoDBComponent::OnMessage(int id, mongo::Request * request, mongo::Response * response) noexcept
	{
		int rpcId = 0;
		if(response == nullptr)
		{
			rpcId = request->GetRpcId();
			LOG_ERROR("send mongo cmd = {}", request->ToString());
		}
		else
		{
			rpcId = response->RpcId();
			if(response->GetCode() != 0)
			{
				LOG_WARN("mongo request = {}", request->ToString());
				LOG_WARN("mongo response = {}", response->ToString());
			}
			else if(this->mConfig.Debug)
			{
				CONSOLE_LOG_INFO("[request] = {}", request->ToString());
				CONSOLE_LOG_INFO("[response:{}] = {}", request->GetCostTime(), response->ToString());
			}
		}
		this->OnResponse(rpcId, std::unique_ptr<mongo::Response>(response));

		if(this->mRequests.empty())
		{
			this->mFreeClients.Push(id);
			return;
		}
		std::unique_ptr<mongo::Request> & request1 = this->mRequests.front();
		{
			this->Send(id, std::move(request1));
			this->mRequests.pop();
		}
	}

	std::unique_ptr<mongo::Response> MongoDBComponent::Run(std::unique_ptr<mongo::Request> request)
	{
		if (request->collectionName.empty())
		{
			request->collectionName = this->mCommand;
		}

		int rpcId = this->BuildRpcId();
		{
			request->header.requestID = rpcId;
			request->dataBase = this->mConfig.DB;
			size_t pos = request->tab.find('.');
			if(pos != std::string::npos)
			{
				request->dataBase = request->tab.substr(0, pos);
				request->collectionName = fmt::format("{}.$cmd", request->dataBase);
			}
		}
		this->Send(std::move(request));
		return this->BuildRpcTask<MongoTask>(rpcId)->Await();
	}

	std::unique_ptr<mongo::Response> MongoDBComponent::Run(const std::string & db, std::unique_ptr<mongo::Request> request)
	{
		int rpcId = this->BuildRpcId();
		{
			request->dataBase = db;
			request->header.requestID = rpcId;
		}
		this->Send(std::move(request));
		return this->BuildRpcTask<MongoTask>(rpcId)->Await();
	}

	void MongoDBComponent::LuaSend(std::unique_ptr<mongo::Request> request, int& rpcId)
	{
//		if(request->collectionName.empty())
//		{
//			request->collectionName = this->mCommand;
//		}
		rpcId = this->BuildRpcId();
		{
			request->header.requestID = rpcId;
			request->dataBase = this->mConfig.DB;
		}
		this->Send(std::move(request));
	}

	void MongoDBComponent::Send(std::unique_ptr<mongo::Request> request)
	{
		int id = 0;
		if(!this->mFreeClients.Pop(id))
		{
			this->mRequests.push(std::move(request));
			return;
		}
		this->Send(id, std::move(request));
	}

	void MongoDBComponent::Send(int id, std::unique_ptr<mongo::Request> request)
	{
		auto iter = this->mMongoClients.find(id);
		if(iter == this->mMongoClients.end())
		{
			LOG_ERROR("mongo[{}] request:{}", id, request->ToString());
			return;
		}
		iter->second->SendMongoCommand(std::move(request));
	}

	void MongoDBComponent::OnRecord(json::w::Document& document)
	{
		std::unique_ptr<json::w::Value> data = document.AddObject("mongo");
		{
			data->Add("sum", this->CurrentRpcCount());
			data->Add("free", this->mFreeClients.Size());
			data->Add("client", this->mMongoClients.size());
			data->Add("wait", this->AwaitCount() + this->mRequests.size());
		}
	}
}