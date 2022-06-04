//
// Created by mac on 2022/5/18.
//

#include"RedisBaseComponent.h"
#include"Util/DirectoryHelper.h"
#include"Component/Scene/NetThreadComponent.h"
namespace Sentry
{
	std::shared_ptr<RedisClientContext> RedisBaseComponent::MakeRedisClient(const RedisConfig* config)
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
		std::shared_ptr<SocketProxy> socketProxy = std::make_shared<SocketProxy>(workThread, ip, port);
		std::shared_ptr<RedisClientContext> redisCommandClient = std::make_shared<RedisClientContext>(socketProxy, config);

		XCode code = redisCommandClient->StartConnect();
		if (code == XCode::RedisAuthFailure)
		{
			LOG_ERROR("redis auth failure");
			return nullptr;
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

	bool RedisBaseComponent::CallLua(const std::string& fullName, Json::Writer& request, const std::string redis)
	{
		std::shared_ptr<Json::Reader> response(new Json::Reader());
		return this->CallLua(fullName, request, response, redis);
	}

	bool RedisBaseComponent::CallLua(const std::string& fullName, Json::Writer& parameter,
			std::shared_ptr<Json::Reader> response, const std::string redis)
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
		std::shared_ptr<RedisClientContext> redisClientContext = this->GetClient(redis);
		if(redisClientContext == nullptr)
		{
			return false;
		}
		std::shared_ptr<RedisResponse> response1(new RedisResponse());
		std::shared_ptr<RedisRequest> request = RedisRequest::MakeLua(tag, func, parameter);
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

	bool RedisBaseComponent::TryReConnect(std::shared_ptr<RedisClientContext> client, int maxCount)
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

	bool RedisBaseComponent::LoadLuaScript(const std::string & redis, const string& path)
	{
		std::string key;
		std::shared_ptr<RedisClientContext> redisClientContext = this->GetClient(redis);
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
