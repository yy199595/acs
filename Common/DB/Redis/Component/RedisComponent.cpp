#include"RedisComponent.h"
#include"Redis/Lua/LuaRedis.h"
#include"Timer/Timer/ElapsedTimer.h"
#include"Lua/Engine/ModuleClass.h"
#include"Server/Component/ThreadComponent.h"
#include"Entity/Actor/App.h"


#include"Util/File/FileHelper.h"
#include"Util/File/DirectoryHelper.h"

namespace joke
{
    bool RedisComponent::Awake()
    {
		std::unique_ptr<json::r::Value> redisObject;
		if(!ServerConfig::Inst()->Get("redis", redisObject))
		{
			return false;
		}
		this->mConfig.Debug = false;
		redisObject->Get("ping", this->mConfig.Ping);
		redisObject->Get("count", this->mConfig.Count);
		redisObject->Get("debug", this->mConfig.Debug);
		redisObject->Get("script", this->mConfig.Script);
		redisObject->Get("passwd", this->mConfig.Password);
		redisObject->Get("address", this->mConfig.Address);
		return true;
    }

	void RedisComponent::OnRecord(json::w::Document& document)
	{
		std::unique_ptr<json::w::Value> data = document.AddObject("redis");
		{
			data->Add("client", this->mClients.size());
			data->Add("free", this->mFreeClients.Size());
			data->Add("sum", this->mNumPool.CurrentNumber());
			data->Add("wait", this->AwaitCount() + this->mRequests.size());
		}
	}

    bool RedisComponent::LateAwake()
	{
		const std::string & address = this->mConfig.Address;
		ThreadComponent* component = this->GetComponent<ThreadComponent>();
		{
			for (int index = 0; index < this->mConfig.Count; index++)
			{
				timer::ElapsedTimer timer1;
				redis::Config config = this->mConfig;
				config.Id = index + 1;
				tcp::Socket * sock = component->CreateSocket(address);
				redis::Client * redisCommandClient = new redis::Client(sock, config, this);
				{
					if(!redisCommandClient->Start())
					{
						LOG_ERROR("start redis [{}] fail count:{}", address, config.Id);
						return false;
					}
					this->mFreeClients.Push(config.Id);
					this->mClients.emplace(config.Id, redisCommandClient);
					LOG_INFO("[{}ms] connect redis [{}] ok", timer1.GetMs(), address);
				}
			}
		}
		return this->LoadRedisScript(this->mConfig.Script);
	}

	void RedisComponent::OnMessage(int id, redis::Request * request, redis::Response * response)
	{
		if (response->HasError())
		{
			LOG_ERROR("redis request = {}", request->ToString());
			LOG_ERROR("redis response = {}", response->ToString());
		}
		else if(this->mConfig.Debug)
		{
			CONSOLE_LOG_DEBUG("[request] = {}", request->ToString());
			CONSOLE_LOG_DEBUG("[response:{}] = {}", request->GetCostTime(), response->ToString());
		}
		int rpcId = request->GetRpcId();
		this->OnResponse(rpcId, response);
		delete request;

		if(this->mRequests.empty())
		{
			this->mFreeClients.Push(id);
			return;
		}
		std::unique_ptr<redis::Request> & request1 = this->mRequests.front();
		{
			this->Send(id, std::move(request1));
			this->mRequests.pop();
		}
	}

    bool RedisComponent::Ping()
    {
        std::unique_ptr<redis::Request> request = redis::Request::Make("PING");
        redis::Response * response = this->Run(std::move(request));
        return response != nullptr && !response->HasError();
    }

    redis::Response * RedisComponent::Run(std::unique_ptr<redis::Request> request)
    {
		int taskId = 0;
		this->Send(std::move(request), taskId);
		return this->AddTask(taskId, new RedisTask(taskId))->Await();
    }

	std::unique_ptr<redis::Response> RedisComponent::SyncRun(std::unique_ptr<redis::Request> request)
	{
		redis::Client * redisCommandClient = nullptr;
		for(auto iter = this->mClients.begin(); iter != this->mClients.end(); iter++)
		{
			redisCommandClient = iter->second;
		}
		return redisCommandClient->Sync(std::move(request));
	}

	void RedisComponent::OnLuaRegister(Lua::ModuleClass &luaRegister)
	{
		luaRegister.AddFunction("Run", Lua::redis::Run);
		luaRegister.AddFunction("Call", Lua::redis::Call);
        luaRegister.AddFunction("Send", Lua::redis::Send);
		luaRegister.AddFunction("Sync", Lua::redis::SyncRun);
		luaRegister.End("db.redis");
    }

	void RedisComponent::Send(std::unique_ptr<redis::Request> request)
	{
		int id = 0;
		if(this->mFreeClients.Pop(id))
		{
			this->Send(id, std::move(request));
			return;
		}
		this->mRequests.push(std::move(request));
	}

	bool RedisComponent::LoadRedisScript(const std::string & dir)
	{
		if(dir.empty()) return true;
		std::vector<std::string> luaFiles;
		help::dir::GetFilePaths(dir, "*.lua", luaFiles);
		for (const std::string& path : luaFiles)
		{
			std::string content, name;
			if (!help::fs::ReadTxtFile(path, content))
			{
				LOG_ERROR("load lua {} fail", path);
				return false;
			}
			if (!help::fs::GetFileName(path, name))
			{
				return false;
			}
			timer::ElapsedTimer timer1;
			std::unique_ptr<redis::Response> response = this->SyncRun("SCRIPT", "LOAD", content);
			{
				LOG_CHECK_RET_FALSE(response != nullptr && !response->HasError());
				if(!this->OnLoadScript(name, response->GetString()))
				{
					LOG_ERROR("load redis script [{}.lua] fail", name);
					return false;
				}
				LOG_DEBUG("[{}ms] load redis script [{}.lua] ok", timer1.GetMs(), name);
			}
		}
		return true;
	}

	bool RedisComponent::OnLoadScript(const std::string& name, const std::string& md5)
	{
		auto iter = this->mLuaMap.find(name);
		if(iter != this->mLuaMap.end())
		{
			return false;
		}
		this->mLuaMap.emplace(name, md5);
		return true;
	}

	bool RedisComponent::Send(const joke::RedisLuaData& data)
	{
		std::unique_ptr<redis::Request> request;
		if (!this->MakeLuaRequest(data, request))
		{
			return false;
		}

		this->Send(std::move(request));
		return true;
	}

	bool RedisComponent::Send(const joke::RedisLuaData& data, int& taskId)
	{
		std::unique_ptr<redis::Request> request;
		if (!this->MakeLuaRequest(data, request))
		{
			return false;
		}
		this->Send(std::move(request), taskId);
		return true;
	}

	redis::Response* RedisComponent::CallLua(const joke::RedisLuaData& data)
	{
		std::unique_ptr<redis::Request> request;
		if(!this->MakeLuaRequest(data, request))
		{
			return nullptr;
		}
		LOG_DEBUG("call [{}] json:{}", data.name, data.json);
		return this->Run(std::move(request));
	}

	std::unique_ptr<json::r::Document> RedisComponent::Call(const joke::RedisLuaData& data)
	{
		std::unique_ptr<redis::Request> request;
		if(!this->MakeLuaRequest(data, request))
		{
			return nullptr;
		}
		LOG_DEBUG("call [{}] json:{}", data.name, data.json);
		redis::Response * response = this->Run(std::move(request));
		if(response == nullptr || response->HasError())
		{
			return nullptr;
		}
		std::unique_ptr<json::r::Document> document = std::make_unique<json::r::Document>();
		if(!document->Decode(response->GetString()))
		{
			return nullptr;
		}
		return document;
	}

	bool RedisComponent::MakeLuaRequest(const joke::RedisLuaData& data, std::unique_ptr<redis::Request>& request)
	{
		const std::string & fullName = data.name;
		size_t pos = fullName.find('.');
		if(pos == std::string::npos)
		{
			return false;
		}
		const std::string tab = fullName.substr(0, pos);
		const std::string func = fullName.substr(pos + 1);
		auto iter = this->mLuaMap.find(tab);
		if(iter == this->mLuaMap.end())
		{
			LOG_ERROR("not find redis script {}", fullName);
			return false;
		}
		const std::string & tag = iter->second;
		request = redis::Request::MakeLua(tag, func, data.json);
		return true;
	}

	void RedisComponent::Send(std::unique_ptr<redis::Request> request, int& id)
	{
		id = this->mNumPool.BuildNumber();
		{
			request->SetRpcId(id);
			this->Send(std::move(request));
		}
	}

	void RedisComponent::Send(int id, std::unique_ptr<redis::Request> request)
	{
		auto iter = this->mClients.find(id);
		if(iter == this->mClients.end())
		{
			LOG_ERROR("redis:{} not exist message:{}", id, request->ToString());
			return;
		}
		//LOG_DEBUG("redis[{}] request:{}", id, request->ToString());
		iter->second->Send(std::move(request));
	}

	void RedisComponent::OnDestroy()
	{
		this->Run("QUIT");
		this->SyncRun("QUIT");
	}

	bool RedisComponent::UnLock(const std::string& key)
	{
		if(key.empty())
		{
			return false;
		}
		RedisLuaData redisLuaData;
		{
			json::w::Document document;
			document.Add("key", key);

			redisLuaData.name = "lock.unlock";
			document.Encode(&redisLuaData.json);
		}
		bool result = false;
		auto response = this->CallLua(redisLuaData);
		return response && response->GetNumber() > 0;
	}

	bool RedisComponent::Lock(const std::string& key, int timeout)
	{
		if(key.empty())
		{
			return false;
		}
		RedisLuaData redisLuaData;
		{
			json::w::Document document;
			document.Add("key", key);
			document.Add("time", timeout);
			redisLuaData.name = "lock.lock";
			document.Encode(&redisLuaData.json);
		}
		auto response = this->CallLua(redisLuaData);
		return response && response->GetNumber() > 0;
	}

	bool RedisComponent::Del(const std::string& key)
	{
		redis::Response * response = this->Run("DEL", key);
		if(response == nullptr || response->HasError())
		{
			return false;
		}
		return response->GetNumber() > 0;
	}
}
