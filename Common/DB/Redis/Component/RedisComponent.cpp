#include"RedisComponent.h"
#include"Timer/Timer/ElapsedTimer.h"
#include "Lua/Lib/Lib.h"
#include"Server/Component/ThreadComponent.h"
#include"Entity/Actor/App.h"


#include"Util/File/FileHelper.h"
#include"Util/File/DirectoryHelper.h"

namespace acs
{
	RedisComponent::RedisComponent()
	{
		REGISTER_JSON_CLASS_FIELD(redis::Config, ping);
		REGISTER_JSON_CLASS_FIELD(redis::Config, count);
		REGISTER_JSON_CLASS_FIELD(redis::Config, debug);
		REGISTER_JSON_CLASS_FIELD(redis::Config, script);
		REGISTER_JSON_CLASS_FIELD(redis::Config, password);
		REGISTER_JSON_CLASS_MUST_FIELD(redis::Config, address);
	}

    bool RedisComponent::Awake()
    {
		LuaCCModuleRegister::Add([](Lua::CCModule & ccModule) {
			ccModule.Open("db.redis", lua::lib::luaopen_lredisdb);
		});
		return ServerConfig::Inst()->Get("redis", this->mConfig);
    }

	void RedisComponent::OnRecord(json::w::Document& document)
	{
		timer::ElapsedTimer timer1;
		this->Run("PING");
		std::unique_ptr<json::w::Value> data = document.AddObject("redis");
		{
			data->Add("sum", this->CurrentRpcCount());
			data->Add("client", this->mClients.size());
			data->Add("free", this->mFreeClients.Size());
			data->Add("ping", fmt::format("{}ms", timer1.GetMs()));
			data->Add("wait", this->AwaitCount() + this->mRequests.size());
		}
	}

    bool RedisComponent::LateAwake()
	{
		const std::string & address = this->mConfig.address;
		ThreadComponent* component = this->GetComponent<ThreadComponent>();
		{
			for (int index = 0; index < this->mConfig.count; index++)
			{
				timer::ElapsedTimer timer1;
				redis::Config config = this->mConfig;
				config.id = index + 1;
				Asio::Context & io = this->mApp->GetContext();
				tcp::Socket * sock = component->CreateSocket(address);
				std::shared_ptr<redis::Client> redisCommandClient = std::make_shared<redis::Client>(sock, config, this, io);
				{
					if(!redisCommandClient->Start())
					{
						LOG_ERROR("connect redis [{}] fail count:{}", address, config.id);
						return false;
					}
					this->mFreeClients.Push(config.id);
					this->mClients.emplace(config.id, redisCommandClient);
					LOG_INFO("[{}ms] connect redis [{}] ok", timer1.GetMs(), address);
				}
			}
		}
		return this->LoadRedisScript(this->mConfig.script);
	}

	void RedisComponent::OnSecondUpdate(int tick) noexcept
	{
		if(tick % this->mConfig.ping == 0)
		{
			int id = 0;
			while(this->mFreeClients.Pop(id))
			{
				std::unique_ptr<redis::Request> request = redis::Request::Make("PING");
				{
					request->SetRpcId(0);
					this->Send(id, std::move(request));
				}
			}
		}
	}

	void RedisComponent::OnMessage(int id, redis::Request * request, redis::Response * response) noexcept
	{
		if (response->HasError())
		{
			LOG_ERROR("redis request = {}", request->ToString());
			LOG_ERROR("redis response = {}", response->ToString());
		}
		else if(this->mConfig.debug)
		{
			CONSOLE_LOG_DEBUG("[request] = {}", request->ToString());
			CONSOLE_LOG_DEBUG("[response:{}ms] = {}", request->GetCostTime(), response->ToString());
		}
		int rpcId = request->GetRpcId();
		if(rpcId > 0)
		{
			this->OnResponse(rpcId, std::unique_ptr<redis::Response>(response));
		}

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
		std::unique_ptr<redis::Response> response = this->Run(std::move(request));
        return response != nullptr && !response->HasError();
    }

	std::unique_ptr<redis::Response> RedisComponent::Run(std::unique_ptr<redis::Request> request) noexcept
    {
		int taskId = 0;
		this->Send(std::move(request), taskId);
		return this->BuildRpcTask<RedisTask>(taskId)->Await();
    }

	std::unique_ptr<redis::Response> RedisComponent::SyncRun(std::unique_ptr<redis::Request> request) noexcept
	{
		for(auto iter = this->mClients.begin(); iter != this->mClients.end(); iter++)
		{
			return iter->second->Sync(std::move(request));
		}
		return nullptr;
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
				LOG_CHECK_RET_FALSE(response != nullptr && response->element.IsString());
				if(!this->OnLoadScript(name, response->element.message))
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

	bool RedisComponent::Send(const acs::RedisLuaData& data) noexcept
	{
		std::unique_ptr<redis::Request> request;
		if (!this->MakeLuaRequest(data, request))
		{
			return false;
		}

		this->Send(std::move(request));
		return true;
	}

	bool RedisComponent::Send(const acs::RedisLuaData& data, int& taskId) noexcept
	{
		std::unique_ptr<redis::Request> request;
		if (!this->MakeLuaRequest(data, request))
		{
			return false;
		}
		this->Send(std::move(request), taskId);
		return true;
	}

	std::unique_ptr<redis::Response> RedisComponent::CallLua(const acs::RedisLuaData& data)
	{
		std::unique_ptr<redis::Request> request;
		if(!this->MakeLuaRequest(data, request))
		{
			return nullptr;
		}
		return this->Run(std::move(request));
	}

	std::unique_ptr<json::r::Document> RedisComponent::Call(const acs::RedisLuaData& data)
	{
		std::unique_ptr<redis::Request> request;
		if(!this->MakeLuaRequest(data, request))
		{
			return nullptr;
		}
		std::unique_ptr<redis::Response> response = this->Run(std::move(request));
		if(response == nullptr || response->element.IsString())
		{
			return nullptr;
		}
		std::unique_ptr<json::r::Document> document = std::make_unique<json::r::Document>();
		if(!document->Decode(response->element.message))
		{
			return nullptr;
		}
		return document;
	}

	bool RedisComponent::MakeLuaRequest(const acs::RedisLuaData& data, std::unique_ptr<redis::Request>& request)
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

	void RedisComponent::Send(std::unique_ptr<redis::Request> request, int& rpcId)
	{
		rpcId = this->BuildRpcId();
		{
			request->SetRpcId(rpcId);
			this->Send(std::move(request));
		}
	}

	void RedisComponent::Send(int id, std::unique_ptr<redis::Request> request) noexcept
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
//		this->Run("QUIT");
//		this->SyncRun("QUIT");
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
		return response && response->element.number > 0;
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
		return response && response->element.number > 0;
	}

	bool RedisComponent::Del(const std::string& key)
	{
		std::unique_ptr<redis::Response> response = this->Run("DEL", key);
		if(response == nullptr || response->HasError())
		{
			return false;
		}
		return response->element.number > 0;
	}
}
