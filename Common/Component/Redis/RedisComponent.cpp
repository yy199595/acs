//
// Created by mac on 2022/5/18.
//

#include"RedisComponent.h"
#include"Util/DirectoryHelper.h"
#include"Component/Scene/NetThreadComponent.h"
namespace Sentry
{
	bool RedisComponent::LateAwake()
	{
		this->mTimerIndex = 0;
		const rapidjson::Value* jsonValue = this->GetApp()->GetConfig().GetJsonValue("redis");
		if (jsonValue == nullptr || !jsonValue->IsObject())
		{
			return false;
		}
		auto iter = jsonValue->MemberBegin();
		for (; iter != jsonValue->MemberEnd(); iter++)
		{
			RedisConfig redisConfig;
			redisConfig.FreeClient = 30;
			const std::string name(iter->name.GetString());
			const rapidjson::Value& jsonData = iter->value;
			redisConfig.Ip = jsonData["ip"].GetString();
			redisConfig.Port = jsonData["port"].GetInt();

			if(jsonData.HasMember("free"))
			{
				redisConfig.FreeClient = jsonData["free"].GetInt();
			}
			if (jsonData.HasMember("passwd"))
			{
				redisConfig.Password = jsonData["passwd"].GetString();
			}
			if (jsonData.HasMember("lua") && jsonData["lua"].IsArray())
			{
				for (int index = 0; index < jsonData["lua"].Size(); index++)
				{
					std::string lua(jsonData["lua"][index].GetString());
					redisConfig.LuaFiles.emplace_back(lua);
				}
			}
			this->mConfigs.emplace(name, redisConfig);
		}
		this->mTaskComponent = this->GetApp()->GetTaskComponent();
		return this->mConfigs.find("main") != this->mConfigs.end();
	}

	bool RedisComponent::OnStart()
	{
		for(auto iter = this->mConfigs.begin(); iter != this->mConfigs.end(); iter++)
		{
			RedisClientContext * redisClientContext = this->MakeRedisClient(&iter->second);
			if(!this->TryAsyncConnect(redisClientContext))
			{
				delete redisClientContext;
				return false;
			}
			for(const std::string & path : iter->second.LuaFiles)
			{
				if(!this->LoadLuaScript(redisClientContext, path))
				{
					delete redisClientContext;
					LOG_ERROR("load script " << path << " error");
					return false;
				}
			}
			this->PushClient(redisClientContext);
		}
		return true;
	}

	bool RedisComponent::TryAsyncConnect(RedisClientContext* redisCommandClient, int maxCount)
	{
		if(redisCommandClient->IsOpen())
		{
			return true;
		}
		const RedisConfig & config = redisCommandClient->GetConfig();
		for(int index = 0; index < maxCount; index++)
		{
			if(redisCommandClient->StartConnect() == XCode::Successful)
			{
				return true;
			}
			LOG_ERROR(config.Name << " connect redis ["
								  << config.Address << "] failure count = " << index);
			this->mTaskComponent->Sleep(3000);
		}
		return false;
	}

	void RedisComponent::OnSecondUpdate()
	{
		this->mTimerIndex++;
		if (this->mTimerIndex >= 10)
		{
			long long nowMs = Helper::Time::GetNowMilTime();
			auto iter = this->mRedisClients.begin();
			for (; iter != this->mRedisClients.end(); iter++)
			{
				for (auto iter1 = iter->second.begin(); iter1 != iter->second.end();)
				{
					RedisClientContext* redisClientContext = (*iter1);
					const RedisConfig& config = redisClientContext->GetConfig();
					if (nowMs - redisClientContext->GetLastOperTime() >= config.FreeClient)
					{
						iter->second.erase(iter1++);
						delete redisClientContext;
						continue;
					}
					iter1++;
				}
			}
		}
	}

	RedisClientContext * RedisComponent::MakeRedisClient(const RedisConfig* config)
	{
#ifdef ONLY_MAIN_THREAD
		IAsioThread& workThread = App::Get()->GetTaskScheduler();
#else
		NetThreadComponent * threadPoolComponent = this->GetComponent<NetThreadComponent>();
		IAsioThread& workThread = threadPoolComponent->AllocateNetThread();
#endif
		const std::string& ip = config->Ip;
		unsigned short port = config->Port;
		std::shared_ptr<SocketProxy> socketProxy(new SocketProxy(workThread, ip, port));
		return new RedisClientContext(socketProxy, *config);
	}

	const RedisConfig* RedisComponent::GetRedisConfig(const std::string& name)
	{
		auto iter = this->mConfigs.find(name);
		return iter != this->mConfigs.end() ? &iter->second : nullptr;
	}

	RedisClientContext* RedisComponent::GetClient(const std::string& name)
	{
		auto iter = this->mRedisClients.find(name);
		if(iter != this->mRedisClients.end())
		{
			if(!iter->second.empty())
			{
				RedisClientContext * clientContext = iter->second.front();
				iter->second.pop_front();
				return clientContext;
			}
		}
		const RedisConfig * config = this->GetRedisConfig(name);
		if(config == nullptr)
		{
			LOG_ERROR("not find redis config : " << name);
			return nullptr;
		}
		return this->MakeRedisClient(config);
	}

	void RedisComponent::PushClient(RedisClientContext* redisClientContext)
	{
		const std::string & name = redisClientContext->GetName();
		auto iter = this->mRedisClients.find(name);
		if(iter == this->mRedisClients.end())
		{
			std::list<RedisClientContext *> clients;
			this->mRedisClients.emplace(name, clients);
		}
		this->mRedisClients[name].emplace_back(redisClientContext);
	}

	bool RedisComponent::Call(const std::string & name, const std::string& fullName, const std::string & json)
	{
		std::shared_ptr<Json::Reader> response(new Json::Reader());
		return this->Call(name, fullName, json, response);
	}

	bool RedisComponent::Call(const std::string & name, const std::string& fullName, const std::string& json,
			std::shared_ptr<Json::Reader> response)
	{
		size_t pos = fullName.find('.');
		assert(pos != std::string::npos);
		std::string tab = fullName.substr(0, pos);
		std::string func = fullName.substr(pos + 1);
		auto iter = this->mLuaMap.find(fmt::format("{0}.lua", tab));
		if(iter == this->mLuaMap.end())
		{
			LOG_ERROR("not find redis script " << fullName);
			return false;
		}
		const std::string & tag = iter->second;
		RedisClientContext * redisClientContext = this->GetClient(name);
		if(redisClientContext == nullptr)
		{
			return false;
		}
		std::shared_ptr<RedisResponse> response1(new RedisResponse());
		std::shared_ptr<RedisRequest> request = RedisRequest::MakeLua(tag, func, json);
		if (redisClientContext->Run(request, response1) != XCode::Successful)
		{
			return false;
		}

		std::string responseJson;
		if(!response1->GetString(responseJson) || !response->ParseJson(responseJson))
		{
			return false;
		}
#ifdef __REDIS_DEBUG__
		std::string json;
		LOG_INFO("=========== call redis lua ===========");
		LOG_INFO("func = " << fullName);
		if(parameter.WriterStream(json))
		{
			LOG_INFO("request = " << json);
		}
		LOG_INFO("response = " << responseJson);
#endif
		bool res = false;
		return response->GetMember("res", res) && res;
	}

	void RedisComponent::OnResponse(std::shared_ptr<RedisResponse> response)
	{
		if(response->GetArraySize() == 3)
		{
			const RedisAny * redisAny1 = response->Get(0);
			const RedisAny * redisAny2 = response->Get(1);
			const RedisAny * redisAny3 = response->Get(2);
			if(redisAny1->IsString() && redisAny2->IsString() && redisAny3->IsString())
			{
				if(static_cast<const RedisString*>(redisAny1)->GetValue() == "message")
				{
					const std::string & channel = static_cast<const RedisString*>(redisAny2)->GetValue();
					const std::string & message = static_cast<const RedisString*>(redisAny3)->GetValue();
					this->OnSubscribe(channel, message);
					return;
				}
			}
		}

	}

	bool RedisComponent::LoadLuaScript(RedisClientContext * redisClientContext, const string& path)
	{
		std::string key;
		if (!redisClientContext->LoadLuaScript(path, key))
		{
			return false;
		}
		std::string fileName, director;
		if (!Helper::Directory::GetDirAndFileName(path, director, fileName))
		{
			return false;
		}
		this->mLuaMap.emplace(fileName, key);
		return true;
	}
}
