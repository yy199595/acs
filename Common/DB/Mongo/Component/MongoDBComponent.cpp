//
// Created by mac on 2022/6/28.
//
#include "MongoDBComponent.h"
#include "Entity/Actor/App.h"
#include "Util/Tools/TimeHelper.h"
#include "Yyjson/Lua/ljson.h"
#include "Lua/Engine/ModuleClass.h"
#include "Timer/Timer/ElapsedTimer.h"
#include "Server/Config/ServerConfig.h"
#include "Server/Component/ThreadComponent.h"
#include "Proto/Component/ProtoComponent.h"
#include "Log/Component/LoggerComponent.h"
// use admin 	=>db.createUser({ user: 'root', pwd: '199595yjz.', roles: [{ role: 'root', db: 'yjz' }] })
// use yjz 		=> db.createUser({ user: 'root', pwd: '199595yjz.', roles: [{ role: 'readWrite', db: 'yjz' }] })

namespace acs
{
	MongoTask::MongoTask(int id)
			: IRpcTask<mongo::Response>(id) { }

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
		if (response != nullptr)
		{
			std::string json;
			if(response->Encode(&json))
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
		this->mSumCount = 0;
		this->mRetryCount = 0;
		REGISTER_JSON_CLASS_FIELD(db::Explain, open);
		REGISTER_JSON_CLASS_FIELD(db::Explain, command);

		REGISTER_JSON_CLASS_FIELD(mongo::Cluster, log);
		REGISTER_JSON_CLASS_FIELD(mongo::Cluster, auth);
		REGISTER_JSON_CLASS_FIELD(mongo::Cluster, ping);
		REGISTER_JSON_CLASS_FIELD(mongo::Cluster, debug);
		REGISTER_JSON_CLASS_FIELD(mongo::Cluster, count);
		REGISTER_JSON_CLASS_FIELD(mongo::Cluster, retry);
		REGISTER_JSON_CLASS_FIELD(mongo::Cluster, explain);

		REGISTER_JSON_CLASS_MUST_FIELD(mongo::Cluster, address);

	}

	bool MongoDBComponent::Awake()
	{
		LuaCCModuleRegister::Add([](Lua::CCModule & ccModule) {
			ccModule.Open("db.mongo", lua::lib::luaopen_lmonogodb);
		});
		ServerConfig & config = this->mApp->GetConfig();
		LOG_CHECK_RET_FALSE(config.Get("mongo", this->mConfig));
		return true;
	}

	bool MongoDBComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(!this->mConfig.address.empty())
		ThreadComponent* threadComponent = this->GetComponent<ThreadComponent>();
		for (int x = 0; x < this->mConfig.address.size(); x++)
		{
			const std::string& address = this->mConfig.address[x];
			for (int index = 0; index < this->mConfig.count && threadComponent; index++)
			{
				mongo::Config config;
				if (!MongoDBComponent::DecodeUrl(address, config))
				{
					return false;
				}
				this->mRetryCount++;
				timer::ElapsedTimer timer1;
				int id = (x + 1) * 100 + index + 1;
				config.conn_count = this->mConfig.conn_count;
				Asio::Context& io = this->mApp->GetContext();
				tcp::Socket* socketProxy = threadComponent->CreateSocket(config.address);
				std::shared_ptr<mongo::Client> mongoClientContext = std::make_unique<mongo::Client>(id, this, config, io);
				{
					if (!mongoClientContext->Start(socketProxy))
					{
						LOG_ERROR("connect {} fail", address);
						return false;
					}
				}
				this->mClients.emplace(id, mongoClientContext);
				LOG_INFO("[{}ms] connect {} ok", timer1.GetMs(), address);

			}
		}
		return true;
	}

	bool MongoDBComponent::DecodeUrl(const std::string& url, mongo::Config& config)
	{
		if(!config.Decode(url))
		{
			return false;
		}
		config.Get("user", config.user);
		config.Get("address", config.address);
		LOG_CHECK_RET_FALSE(config.Get("db", config.db))
		LOG_CHECK_RET_FALSE(config.Get("address", config.address))
		if(!config.Get("mechanism", config.mechanism))
		{
			config.mechanism = mongo::auth::SCRAM_SHA1;
		}
#ifdef __ENABLE_OPEN_SSL__
		return config.mechanism == mongo::auth::SCRAM_SHA1
			|| config.mechanism == mongo::auth::SCRAM_SHA256;
#else
		return config.mechanism == mongo::auth::SCRAM_SHA1;
#endif
	}

	void MongoDBComponent::OnConnectOK(int id)
	{
		this->mRetryCount--;
		if(this->mClients.find(id) != this->mClients.end())
		{
			this->AddFreeClient(id);
			LOG_DEBUG("mongo client:{} login ok", id);
		}
	}

	void MongoDBComponent::OnClientError(int id, int code)
	{
		auto iter = this->mClients.find(id);
		if(iter == this->mClients.end())
		{
			return;
		}
		this->mRetryCount--;
		this->mRetryClients.emplace(id);
		std::string address = iter->second->GetAddress();
		LOG_WARN("mongo client ({}) login fail", address)
	}

	void MongoDBComponent::OnSendFailure(int id, mongo::Request* message)
	{
		this->Send(std::unique_ptr<mongo::Request>(message));
		LOG_WARN("mongo client:{} resend => {}", id, message->ToString())
	}

	void MongoDBComponent::OnDestroy()
	{
		auto iter = this->mClients.begin();
		for(; iter != this->mClients.end(); iter++)
		{
			iter->second->Stop();
		}
		this->mClients.clear();
	}

	void MongoDBComponent::OnMessage(int id, mongo::Request * request, mongo::Response * response) noexcept
	{
		this->mSumCount++;
		this->AddFreeClient(id);
		if (response == nullptr)
		{
			LOG_FATAL("send mongo cmd = {}", request->ToString());
			return;
		}

		int rpcId = response->RpcId();
		if (response->GetCode() != 0)
		{
			LOG_WARN("mongo request = {}", request->ToString());
			LOG_WARN("mongo response = {}", response->ToString());
		}
		else
		{
			if(!response->Document().IsOk())
			{
				std::string errmsg;
				LOG_WARN("request => {}", request->ToString());
				if(response->Document().Get("errmsg", errmsg))
				{
					LOG_WARN("response => {}", errmsg);
				}
			}
			if(this->mConfig.explain.open && request->cmd.find("explain") == std::string::npos
			   && this->mConfig.explain.HasCommand(request->cmd))
			{
				std::unique_ptr<mongo::Request> newRequest = std::make_unique<mongo::Request>();
				{
					newRequest->cmd = "explain";
					newRequest->dataBase = request->dataBase;
					newRequest->document.Add("explain", request->document);
				}
				long long ms = request->GetCostTime();
				this->mApp->StartCoroutine([req = newRequest.release(), ms, this] {
					this->OnExplain(std::unique_ptr<mongo::Request>(req), ms);
				});
			}
			if(this->mConfig.debug)
			{
				CONSOLE_LOG_INFO("[request] = {}", request->ToString());
				CONSOLE_LOG_INFO("[response:{}] = {}", request->GetCostTime(), response->ToString());
			}
		}

		if(rpcId > 0)
		{
			this->OnResponse(rpcId, std::unique_ptr<mongo::Response>(response));
		}
	}

	void MongoDBComponent::OnSecondUpdate(int tick) noexcept
	{
		if(this->mConfig.ping > 0 && tick % this->mConfig.ping == 0)
		{
			int id = 0;
			while(this->mFreeClients.Pop(id))
			{
				std::unique_ptr<mongo::Request> request = std::make_unique<mongo::Request>();
				{
					request->cmd = "ping";
					request->header.requestID = 0;
					request->document.Add("ping", 1);
				}
				this->Send(id, std::move(request));
				//CONSOLE_LOG_WARN("mongodb client:{} ping", id)
			}
		}
		if(this->mConfig.retry > 0 && tick % this->mConfig.retry == 0)
		{
			for(const int id : this->mRetryClients)
			{
				auto iter = this->mClients.find(id);
				if(iter != this->mClients.end())
				{
					iter->second->Start(nullptr);
				}
			}
			this->mRetryClients.clear();
		}
	}

	void MongoDBComponent::OnExplain(std::unique_ptr<mongo::Request> request, long long ms) noexcept
	{
		std::unique_ptr<custom::LogInfo> sqlLog = std::make_unique<custom::LogInfo>();
		{
			json::r::Document readDocument;
			if (readDocument.Decode(request->document.ToString()))
			{
				std::unique_ptr<json::r::Value> jsonValue;
				if (readDocument.Get("explain", jsonValue))
				{
					sqlLog->Content.assign(fmt::format("[{}ms] ", ms));
					sqlLog->Content.append(jsonValue->ToString());
					sqlLog->Content.append("\n");
				}
			}
		}
		LoggerComponent * logger = this->GetComponent<LoggerComponent>();
		std::unique_ptr<mongo::Response> response = this->Run(std::move(request));
		if (response != nullptr)
		{
			sqlLog->Level = custom::LogLevel::None;
			sqlLog->Content.append(response->ToString());
			logger->PushLog("mongo", std::move(sqlLog));
		}
	}

	std::unique_ptr<mongo::Response> MongoDBComponent::Run(std::unique_ptr<mongo::Request> request)
	{
		int rpcId = 0;
		this->Send(std::move(request), rpcId);
		return this->BuildRpcTask<MongoTask>(rpcId)->Await();
	}

	std::unique_ptr<mongo::Response> MongoDBComponent::Run(const std::string & db, std::unique_ptr<mongo::Request> request)
	{
		int rpcId = 0;
		request->dataBase = db;
		this->Send(std::move(request), rpcId);
		return this->BuildRpcTask<MongoTask>(rpcId)->Await();
	}

	void MongoDBComponent::Send(std::unique_ptr<mongo::Request> request, int& rpcId)
	{
		rpcId = this->BuildRpcId();
		{
			request->header.requestID = rpcId;
		}
		this->Send(std::move(request));
	}

	void MongoDBComponent::AddFreeClient(int id)
	{
		if(!this->mRequests.empty())
		{
			this->Send(id, std::move(this->mRequests.front()));
			this->mRequests.pop();
			return;
		}
		this->mFreeClients.Push(id);
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
		auto iter = this->mClients.find(id);
		if(iter == this->mClients.end())
		{
			LOG_ERROR("mongo[{}] request:{}", id, request->ToString());
			return;
		}
		iter->second->SendMongoCommand(std::move(request));
	}

	void MongoDBComponent::OnRecord(json::w::Document& document)
	{
		timer::ElapsedTimer timer1;
		std::unique_ptr<mongo::Request> request = std::make_unique<mongo::Request>();
		{
			request->cmd = "ping";
			request->header.requestID = this->BuildRpcId();
			request->document.Add("ping", 1);
		}
		this->Run(std::move(request));
		std::unique_ptr<json::w::Value> data = document.AddObject("mongo");
		{
			data->Add("retry", this->mRetryCount);
			data->Add("sum", this->CurrentRpcCount());
			data->Add("free", this->mFreeClients.Size());
			data->Add("client", this->mClients.size());
			data->Add("ping", fmt::format("{}ms", timer1.GetMs()));
			data->Add("wait", this->AwaitCount() + this->mRequests.size());
		}
	}
}