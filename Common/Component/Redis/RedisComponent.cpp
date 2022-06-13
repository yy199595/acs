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
		const rapidjson::Value* jsonValue = this->GetApp()->GetConfig().GetJsonValue("redis");
		if (jsonValue == nullptr || !jsonValue->IsObject())
		{
			return false;
		}
		auto iter = jsonValue->MemberBegin();
		for (; iter != jsonValue->MemberEnd(); iter++)
		{
			RedisConfig redisConfig;
			const std::string name(iter->name.GetString());
			const rapidjson::Value& jsonData = iter->value;
			redisConfig.Ip = jsonData["ip"].GetString();
			redisConfig.Port = jsonData["port"].GetInt();
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
		return this->mConfigs.find("main") != this->mConfigs.end();
	}

	bool RedisComponent::OnStart()
	{
		for(auto iter = this->mConfigs.begin(); iter != this->mConfigs.end(); iter++)
		{
			RedisClientContext * redisClientContext = this->MakeRedisClient(&iter->second);
			if(redisClientContext == nullptr)
			{
				return false;
			}
			this->PushClient(redisClientContext);
		}
		return true;
	}

	bool RedisComponent::TryAsyncConnect(RedisClientContext* client, int maxCount)
	{
		XCode code = redisCommandClient->StartConnect();
		if (code == XCode::RedisAuthFailure)
		{
			LOG_ERROR("redis auth failure");
			return false;
		}

		while (code != XCode::Successful)
		{
			LOG_ERROR(config->Name << " connect redis ["
								   << config->Address << "] failure count = " << count++);
			this->GetApp()->GetTaskComponent()->Sleep(3000);
			code = redisCommandClient->StartConnect();
		}
		LOG_INFO(config->Name << " connect redis [" << config->Address << "] successful");
		return redisCommandClient;
	}

	RedisClientContext * RedisComponent::MakeRedisClient(const RedisConfig* config)
	{
#ifdef ONLY_MAIN_THREAD
		IAsioThread& workThread = App::Get()->GetTaskScheduler();
#else
		NetThreadComponent * threadPoolComponent = this->GetComponent<NetThreadComponent>();
		IAsioThread& workThread = threadPoolComponent->AllocateNetThread();
#endif
		size_t count = 0;
		const std::string& ip = config->Ip;
		unsigned short port = config->Port;
		std::shared_ptr<SocketProxy> socketProxy(new SocketProxy(workThread, ip, port));
		RedisClientContext * redisCommandClient = new RedisClientContext(socketProxy, config);
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
				iter->second.pop();
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
			std::queue<RedisClientContext *> clients;
			this->mRedisClients.emplace(name, clients);
		}
		this->mRedisClients[name].push(redisClientContext);
	}

	bool RedisComponent::CallLua(const std::string & name, const std::string& fullName, const std::string & json)
	{
		std::shared_ptr<Json::Reader> response(new Json::Reader());
		return this->CallLua(name, fullName, json, response);
	}

	bool RedisComponent::CallLua(const std::string & name, const std::string& fullName, const std::string& json,
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

	bool RedisComponent::TryReConnect(RedisClientContext * client, int maxCount)
	{
		int count = 0;
		XCode code = client->StartConnect();
		if (code == XCode::RedisAuthFailure)
		{
			LOG_FATAL("redis client auth failure");
			return false;
		}
		while (code != XCode::Successful)
		{
			if(maxCount != 0 && count >= maxCount)
			{
				return false;
			}
			this->GetApp()->GetTaskComponent()->Sleep(3000);
			code = client->StartConnect();
		}
		return true;
	}

	bool RedisComponent::LoadLuaScript(const std::string & redis, const string& path)
	{
		std::string key;
		RedisClientContext * redisClientContext = this->GetClient(redis);
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
		LOG_INFO(redis << "load redis script [" << path << "] successful");
		return true;
	}
}
