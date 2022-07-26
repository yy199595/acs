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
		return this->mConfigs.find("main") != this->mConfigs.end();
	}

	bool RedisComponent::OnStart()
	{
		for(auto iter = this->mConfigs.begin(); iter != this->mConfigs.end(); iter++)
		{
			const RedisConfig & config = iter->second;
			if(this->Run(config.Name, "PING")->HasError())
			{
				LOG_ERROR("connect " << config.Name << " failure");
				return false;
			}
			for(const std::string & path : iter->second.LuaFiles)
			{
				if(!this->LoadLuaScript(config.Name, path))
				{
					LOG_ERROR("load script " << path << " error");
					return false;
				}
			}
		}
		return true;
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
#ifdef ONLY_MAIN_THREAD
		IAsioThread& workThread = App::Get()->GetTaskScheduler();
#else
		NetThreadComponent * threadPoolComponent = this->GetComponent<NetThreadComponent>();
		IAsioThread& workThread = threadPoolComponent->AllocateNetThread();
#endif
		unsigned short port = config.Port;
        const std::string& ip = config.Ip;
        std::shared_ptr<SocketProxy> socketProxy(new SocketProxy(workThread, ip, port));
		return std::make_shared<RedisClientContext>(socketProxy, config, this);
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

	bool RedisComponent::Call(const std::string & name, const std::string& fullName, Json::Writer & jsonWriter)
	{
		std::shared_ptr<Json::Reader> response(new Json::Reader());
		return this->Call(name, fullName, jsonWriter, response);
	}

    std::shared_ptr<RedisRequest> RedisComponent::MakeLuaRequest(
            const std::string &fullName, const std::string &json)
    {
        size_t pos = fullName.find('.');
        assert(pos != std::string::npos);
        std::string tab = fullName.substr(0, pos);
        std::string func = fullName.substr(pos + 1);
        auto iter = this->mLuaMap.find(fmt::format("{0}.lua", tab));
        if(iter == this->mLuaMap.end())
        {
            LOG_ERROR("not find redis script " << fullName);
            return nullptr;
        }
        const std::string & tag = iter->second;
        return RedisRequest::MakeLua(tag, func, json);
    }

	bool RedisComponent::Call(const std::string & name, const std::string& fullName, Json::Writer & jsonWriter,
			std::shared_ptr<Json::Reader> response)
	{
        std::string json;
        jsonWriter.WriterStream(json);
        std::shared_ptr<RedisRequest> request = this->MakeLuaRequest(fullName, json);
        if(request == nullptr)
        {
            return false;
        }
		std::shared_ptr<RedisResponse> response1 = this->Run(name, request);
        if(response1->HasError())
        {
            LOG_ERROR(response1->GetString());
            return false;
        }
        if(!response->ParseJson(response1->GetString()))
        {
            return false;
        }
		bool res = false;
		return response->GetMember("res", res) && res;
	}

    std::shared_ptr<RedisResponse> RedisComponent::Run(
            SharedRedisClient redisClientContext, std::shared_ptr<RedisRequest> request)
    {
#ifdef __DEBUG__
        ElapsedTimer elapsedTimer;
#endif
		std::shared_ptr<RedisTask> redisTask = this->AddRedisTask(request);
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

	void RedisComponent::OnResponse(SharedRedisClient redisClient, long long taskId, std::shared_ptr<RedisResponse> response)
	{
		if(response->GetType() == RedisRespType::REDIS_ERROR)
		{
			LOG_ERROR(response->GetString());
		}
        if(response->GetType() == RedisRespType::REDIS_ARRAY && response->GetArraySize() == 3)
		{
			const RedisAny * redisAny1 = response->Get(0);
			const RedisAny * redisAny2 = response->Get(1);
			const RedisAny * redisAny3 = response->Get(2);
			if(redisAny1->IsString() && redisAny2->IsString() && redisAny3->IsString())
			{
				if(static_cast<const RedisString*>(redisAny1)->GetValue() == "message")
				{
					const std::string & channel = redisAny2->Cast<RedisString>()->GetValue();
					const std::string & message = redisAny3->Cast<RedisString>()->GetValue();
					this->OnSubscribe(redisClient, channel, message);
					return;
				}
			}
		}
        if(taskId != 0)
        {
            this->OnCommandReply(redisClient, taskId, response);
        }
	}

	bool RedisComponent::LoadLuaScript(const std::string & name, const string& path)
	{
        std::string content;
        if (!Helper::File::ReadTxtFile(path, content))
        {
            LOG_ERROR("read " << path << " failure");
            return false;
        }

        std::shared_ptr<RedisResponse> redisResponse =
			this->Run(name, "SCRIPT", "LOAD", content);
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
