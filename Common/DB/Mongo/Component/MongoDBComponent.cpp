//
// Created by mac on 2022/6/28.
//
#include"MongoDBComponent.h"
#include"Entity/Actor/App.h"
#include"Util/Time/TimeHelper.h"
#include"Mongo/Lua/LuaMongo.h"
#include"Yyjson/Lua/ljson.h"
#include"Util/String/String.h"
#include"Lua/Engine/ModuleClass.h"
#include"Timer/Timer/ElapsedTimer.h"
#include"Server/Config/ServerConfig.h"
#include"Server/Component/ThreadComponent.h"

// db.createUser({ user: 'root', pwd: '199595yjz.', roles: [{ role: 'root', db: 'yjz' }] })
namespace joke
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

	int LuaMongoTask::Await()
	{
		if(this->mRef == 0)
		{
			luaL_error(this->mLua, "not lua coroutine context yield failure");
			return 0;
		}
		return lua_yield(this->mLua, 0);
	}

	void LuaMongoTask::OnResponse(mongo::Response * response)
	{
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		lua_State* coroutine = lua_tothread(this->mLua, -1);
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
		Lua::Coroutine::Resume(coroutine, this->mLua, 1);
	}
}


namespace joke
{
	bool MongoDBComponent::Awake()
	{
		std::unique_ptr<json::r::Value> mongoObject;
		if(!ServerConfig::Inst()->Get("mongo", mongoObject))
		{
			return false;
		}
		mongoObject->Get("db", this->mConfig.DB);
		mongoObject->Get("ping", this->mConfig.Ping);
		mongoObject->Get("user", this->mConfig.User);
		mongoObject->Get("debug", this->mConfig.Debug);
		mongoObject->Get("count", this->mConfig.MaxCount);
		mongoObject->Get("address", this->mConfig.Address);
		mongoObject->Get("passwd", this->mConfig.Password);
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
				tcp::Socket * socketProxy = threadComponent->CreateSocket(this->mConfig.Address);
				mongo::Client * mongoClientContext = new mongo::Client(socketProxy, this, config);
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

	void MongoDBComponent::OnMessage(int id, mongo::Request * request, mongo::Response * response)
	{
		int taskId = 0;
		if(response == nullptr)
		{
			taskId = request->GetRpcId();
			LOG_ERROR("send mongo cmd = {}", request->ToString());
		}
		else
		{
			taskId = response->RpcId();
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
		this->OnResponse(taskId, response);
		delete request;

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

	mongo::Response * MongoDBComponent::Run(std::unique_ptr<mongo::Request> request)
	{
		if (request->collectionName.empty())
		{
			request->collectionName = this->mCommand;
		}

		int taskId = this->mNumberPool.BuildNumber();
		{
			request->header.requestID = taskId;
			request->dataBase = this->mConfig.DB;
			size_t pos = request->tab.find('.');
			if(pos != std::string::npos)
			{
				request->dataBase = request->tab.substr(0, pos);
				request->collectionName = fmt::format("{}.$cmd", request->dataBase);
			}
		}
		this->Send(std::move(request));
		return this->AddTask(taskId, new MongoTask(taskId))->Await();
	}

	mongo::Response * MongoDBComponent::Run(const std::string & db, std::unique_ptr<mongo::Request> request)
	{
		int taskId = this->mNumberPool.BuildNumber();
		{
			request->dataBase = db;
			request->header.requestID = taskId;
		}
		this->Send(std::move(request));
		return this->AddTask(taskId, new MongoTask(taskId))->Await();
	}

	void MongoDBComponent::LuaSend(std::unique_ptr<mongo::Request> request, int& taskId)
	{
//		if(request->collectionName.empty())
//		{
//			request->collectionName = this->mCommand;
//		}
		taskId = this->mNumberPool.BuildNumber();
		{
			request->header.requestID = taskId;
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
		//LOG_DEBUG("mongo[{}] request:{}", id, request->ToString());
		iter->second->SendMongoCommand(std::move(request));
	}

	void MongoDBComponent::OnRecord(json::w::Document& document)
	{
		std::unique_ptr<json::w::Value> data = document.AddObject("mongo");
		{
			data->Add("free", this->mFreeClients.Size());
			data->Add("client", this->mMongoClients.size());
			data->Add("sum", (int)this->mNumberPool.CurrentNumber());
			data->Add("wait", this->AwaitCount() + this->mRequests.size());
		}
	}

	void MongoDBComponent::OnLuaRegister(Lua::ModuleClass& luaRegister)
	{
		luaRegister.AddFunction("Run", Lua::LuaMongo::Run);
		luaRegister.End("db.mongo");
	}
}