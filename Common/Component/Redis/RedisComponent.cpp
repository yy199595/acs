//
// Created by mac on 2022/5/18.
//

#include"RedisComponent.h"
#include"Util/FileHelper.h"
#include"Other/ElapsedTimer.h"
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
			redisConfig.FreeClient = 30;
            redisConfig.Name = iter->name.GetString();
			const std::string name(iter->name.GetString());
			const rapidjson::Value& jsonData = iter->value;
			redisConfig.Ip = jsonData["ip"].GetString();
			redisConfig.Port = jsonData["port"].GetInt();
            redisConfig.Index = jsonData["index"].GetInt();
            redisConfig.Count = jsonData["count"].GetInt();
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
            redisConfig.Address = fmt::format("{0}:{1}", redisConfig.Ip, redisConfig.Port);
			this->mConfigs.emplace(name, redisConfig);
		}
		this->mTaskComponent = this->GetApp()->GetTaskComponent();
        this->mNetComponent = this->GetComponent<NetThreadComponent>();
		return this->mConfigs.find("main") != this->mConfigs.end();
	}

    const RedisConfig * RedisComponent::ParseConfig(const char *name, const rapidjson::Value &jsonData)
    {
        auto iter = this->mConfigs.find(name);
        if(iter != this->mConfigs.end())
        {
            return &iter->second;
        }

        LOG_CHECK_RET_NULL(jsonData.HasMember("ip"));
        LOG_CHECK_RET_NULL(jsonData.HasMember("port"));
        LOG_CHECK_RET_NULL(jsonData.HasMember("index"));
        LOG_CHECK_RET_NULL(jsonData.HasMember("count"));

        RedisConfig redisConfig;
        redisConfig.Name = name;
        redisConfig.FreeClient = 30;
        redisConfig.Ip = jsonData["ip"].GetString();
        redisConfig.Port = jsonData["port"].GetInt();
        redisConfig.Index = jsonData["index"].GetInt();
        redisConfig.Count = jsonData["count"].GetInt();
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
        redisConfig.Address = fmt::format("{0}:{1}", redisConfig.Ip, redisConfig.Port);
        this->mConfigs.emplace(name, redisConfig);
        return &this->mConfigs[name];
    }

	void RedisComponent::OnSecondUpdate(const int tick)
	{
        if(tick % 10 == 0)
        {
            long long nowMs = Helper::Time::GetNowMilTime();
            auto iter = this->mRedisClients.begin();
            for (; iter != this->mRedisClients.end(); iter++)
            {
                LOG_DEBUG(iter->first << " redis client count = " << iter->second.size());
                for (auto iter1 = iter->second.begin(); iter1 != iter->second.end();)
                {
                    SharedRedisClient redisClientContext = (*iter1);
                    const RedisConfig& config = redisClientContext->GetConfig();
                    if (nowMs - redisClientContext->GetLastOperTime() >= config.FreeClient)
                    {
                        iter->second.erase(iter1++);
                        continue;
                    }
                    iter1++;
                }
            }
        }
	}

    SharedRedisClient RedisComponent::MakeRedisClient(const std::string &name)
    {
        const RedisConfig * redisConfig = this->GetRedisConfig(name);
        if(redisConfig == nullptr)
        {
            return nullptr;
        }
        return this->MakeRedisClient(*redisConfig);
    }

    SharedRedisClient RedisComponent::MakeRedisClient(const RedisConfig & config)
	{
        std::shared_ptr<SocketProxy> socketProxy = this->mNetComponent->CreateSocket();
        if(socketProxy == nullptr)
        {
            return nullptr;
        }
        socketProxy->Init(config.Ip, config.Port);
		return std::make_shared<RedisClientContext>(socketProxy, config);
	}

	const RedisConfig* RedisComponent::GetRedisConfig(const std::string& name)
	{
		auto iter = this->mConfigs.find(name);
		return iter != this->mConfigs.end() ? &iter->second : nullptr;
	}

    SharedRedisClient RedisComponent::GetClient(const std::string& name)
	{
		auto iter = this->mRedisClients.find(name);
		if(iter != this->mRedisClients.end())
		{
            std::list<SharedRedisClient> & redisClients = iter->second;
            if(!redisClients.empty())
            {
                SharedRedisClient redisClient = redisClients.front();
                redisClients.push_back(redisClient);
                redisClients.pop_front();
                return redisClient;
            }
		}
        SharedRedisClient redisClientContext = this->MakeRedisClient(name);
        if(redisClientContext != nullptr)
        {
            this->PushClient(redisClientContext);
        }
        return redisClientContext;
	}

	void RedisComponent::PushClient(SharedRedisClient redisClientContext)
	{
		const std::string & name = redisClientContext->GetName();
		auto iter = this->mRedisClients.find(name);
		if(iter == this->mRedisClients.end())
		{
			std::list<SharedRedisClient> clients;
			this->mRedisClients.emplace(name, clients);
		}
        LOG_DEBUG("add redis client " << name << " " << redisClientContext.get());
		this->mRedisClients[name].emplace_back(redisClientContext);
	}


    std::shared_ptr<RedisResponse> RedisComponent::Run(
            SharedRedisClient redisClientContext, std::shared_ptr<RedisRequest> request)
    {
#ifdef __DEBUG__
        ElapsedTimer elapsedTimer;
#endif
		std::shared_ptr<RedisTask> redisTask = request->MakeTask();
        this->AddTask(redisTask);
		redisClientContext->SendCommand(request);
        std::shared_ptr<RedisResponse> redisResponse = redisTask->Await();
#ifdef __DEBUG__
        LOG_INFO(request->GetCommand() << " use time = [" << elapsedTimer.GetMs() << "ms]");
#endif
        if (redisResponse->HasError())
        {
            LOG_ERROR(request->ToJson());
            LOG_ERROR(redisResponse->GetString());
        }

        return redisResponse;
    }

    std::shared_ptr<RedisResponse> RedisComponent::Run(const std::string &name, std::shared_ptr<RedisRequest> request)
    {
        SharedRedisClient redisClientContext = this->GetClient(name);
        if(redisClientContext == nullptr)
        {
            return nullptr;
        }
        return this->Run(redisClientContext, request);
    }

	bool RedisComponent::LoadLuaScript(const std::string & name, const string& path)
	{
        std::string content;
        if (!Helper::File::ReadTxtFile(path, content))
        {
            LOG_ERROR("read " << path << " failure");
            return false;
        }
        std::shared_ptr<RedisRequest> request = RedisRequest::Make("SCRIPT", "LOAD", content);

        std::shared_ptr<RedisResponse> redisResponse = this->Run(name, request);
        if(!redisResponse->HasError())
        {
            std::string fileName, director;
            if (!Helper::Directory::GetDirAndFileName(path, director, fileName))
            {
                return false;
            }
            this->mLuaMap.emplace(fileName, redisResponse->GetString());
            return true;
        }
        return false;
	}
}
