#include"MainRedisComponent.h"

#include"App/App.h"
#include"Util/DirectoryHelper.h"
#include"Script/ClassProxyHelper.h"
#include"Global/ServiceConfig.h"
#include"DB/Redis/RedisClientContext.h"
#include"Script/Extension/Redis/LuaRedis.h"
#include"Component/Scene/NetThreadComponent.h"
#include"Component/Rpc/ServiceRpcComponent.h"

namespace Sentry
{
	bool MainRedisComponent::LateAwake()
	{
        RedisComponent::LateAwake();
		this->mTaskComponent = this->GetComponent<TaskComponent>();
		this->mTimerComponent = this->GetComponent<TimerComponent>();
		LOG_CHECK_RET_FALSE(this->GetComponent<NetThreadComponent>());
		this->mRpcTaskComponent = this->GetComponent<ServiceRpcComponent>();
		return true;
	}

	bool MainRedisComponent::OnStart()
	{
        const rapidjson::Value* jsonValue = this->GetApp()->GetConfig().GetJsonValue("redis");
        if (jsonValue == nullptr || !jsonValue->IsObject())
        {
            return false;
        }
        auto iter = jsonValue->MemberBegin();
        for (; iter != jsonValue->MemberEnd(); iter++)
        {
            const char * name = iter->name.GetString();
            if(this->ParseConfig(name, iter->value) == nullptr)
            {
                LOG_ERROR("check " << name << " redis config error");
                return false;
            }
            if(this->RunCmd(name, "PING")->HasError())
            {
                return false;
            }
        }
        return true;
	}

    void MainRedisComponent::OnSecondUpdate(const int tick)
    {
        if (tick % 10 == 0 && this->mSubRedisClient != nullptr)
		{
			this->mSubRedisClient->SendCommand(std::make_shared<RedisRequest>("PING"));
		}
    }

	bool MainRedisComponent::Lock(const string& key, int timeout)
	{
		Json::Writer jsonWriter;
		jsonWriter << "key" << key << "time" << timeout;
		if(!this->Call("main", "lock.lock", jsonWriter))
		{
			return false;
		}
		LOG_DEBUG("redis lock " << key << " get successful");
		this->mLockTimers[key] = this->mTimerComponent->DelayCall(
				(float)timeout - 0.5f, &MainRedisComponent::OnLockTimeout, this, key, timeout);
		return true;
	}

	bool MainRedisComponent::UnLock(const string& key)
	{
		std::shared_ptr<RedisResponse> response1 = this->RunCmd("main", "DEL", key);
        if(response1->GetNumber() != 1)
        {
            return false;
        }

		auto iter = this->mLockTimers.find(key);
		if(iter != this->mLockTimers.end())
		{
			unsigned int id = iter->second;
			this->mLockTimers.erase(iter);
			this->mTimerComponent->CancelTimer(id);
		}
		LOG_INFO(key << " unlock successful");
		return true;
	}

	void MainRedisComponent::OnLockTimeout(const std::string& key, int timeout)
	{
		auto iter = this->mLockTimers.find(key);
		if (iter != this->mLockTimers.end())
		{
			this->mLockTimers.erase(iter);
			this->mTaskComponent->Start([this, key, timeout]()
			{
				std::shared_ptr<RedisResponse> response = this->RunCmd("main", "SETEX", 10, 1);
                if(response != nullptr && response->IsOk())
                {
                    this->mLockTimers[key] = this->mTimerComponent->DelayCall(
                            (float)timeout - 0.5f, &MainRedisComponent::OnLockTimeout,this, key, timeout);
                    return;
                }
				LOG_ERROR("redis lock " << key << " delay failure");
			});
		}
	}
	void MainRedisComponent::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{
		luaRegister.BeginRegister<MainRedisComponent>();
		luaRegister.PushExtensionFunction("Run", Lua::Redis::Run);
		luaRegister.PushExtensionFunction("Call", Lua::Redis::Call);
	}
}

namespace Sentry
{
    bool MainRedisComponent::Call(const std::string & name, const std::string& fullName, Json::Writer & jsonWriter)
    {
        std::shared_ptr<Json::Reader> response(new Json::Reader());
        return this->Call(name, fullName, jsonWriter, response);
    }

    std::shared_ptr<RedisRequest> MainRedisComponent::MakeLuaRequest(const std::string &fullName, const std::string &json)
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

    void MainRedisComponent::OnLoadScript(const std::string &name, const std::string &md5)
    {
        auto iter = this->mLuaMap.find(name);
        if(iter != this->mLuaMap.end())
        {
            this->mLuaMap.erase(iter);
        }
        this->mLuaMap[name] = md5;
    }

    bool MainRedisComponent::Call(const std::string & name, const std::string& fullName, Json::Writer & jsonWriter,
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
}
