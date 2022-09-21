#include"RedisDataComponent.h"

#include"App/App.h"
#include"lua/LuaRedis.h"
#include"String/StringHelper.h"
#include"File/DirectoryHelper.h"
#include"Lua/ClassProxyHelper.h"
#include"Client/TcpRedisClient.h"
#include"Component/NetThreadComponent.h"

namespace Sentry
{
    bool RedisDataComponent::OnInitRedisClient(RedisConfig config)
    {
        config.Channels.clear();
        for (int index = 0; index < config.Count; index++)
        {
            SharedRedisClient redisClient = this->MakeRedisClient(config);
            if(redisClient == nullptr)
            {
                return false;
            }
            if(index == 0 && !this->Ping(redisClient))
            {
                return false;
            }
        }
        return true;
    }

    long long RedisDataComponent::AddCounter(const std::string &id)
    {
        SharedRedisClient redisClientContext = this->GetClient("main");
        std::shared_ptr<RedisResponse> redisResponse =
                this->RunCommand(redisClientContext, "INCR", id);
        if (redisResponse == nullptr || redisResponse->HasError())
        {
            return -1;
        }
        return redisResponse->GetNumber();
    }

    long long RedisDataComponent::SubCounter(const std::string &id)
    {
        SharedRedisClient redisClientContext = this->GetClient("main");
        std::shared_ptr<RedisResponse> redisResponse =
                this->RunCommand(redisClientContext, "DECR", id);
        if (redisResponse == nullptr || redisResponse->HasError())
        {
            return -1;
        }
        return redisResponse->GetNumber();
    }


    bool RedisDataComponent::Call(const std::string &name, const std::string &func, Json::Writer &request,
                                  std::shared_ptr<Json::Reader> response)
    {
        SharedRedisClient redisClientContext = this->GetClient(name);
        std::shared_ptr<RedisRequest> redisRequets = this->MakeLuaRequest(func, request.JsonString());
        if(redisRequets == nullptr)
        {
            return false;
        }
        std::shared_ptr<RedisResponse> redisResponse = this->Run(redisClientContext, redisRequets);
        if (redisResponse == nullptr || redisResponse->HasError())
        {
            return false;
        }
#ifdef __DEBUG__
        LOG_DEBUG("call lua " << func << " response json = " << redisResponse->GetString());
#endif
        return response->ParseJson(redisResponse->GetString());
    }

	void RedisDataComponent::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{
		luaRegister.BeginRegister<RedisDataComponent>();
		luaRegister.PushExtensionFunction("Run", Lua::Redis::Run);
		luaRegister.PushExtensionFunction("Call", Lua::Redis::Call);
        luaRegister.PushExtensionFunction("Send", Lua::Redis::Send);
    }
}

namespace Sentry
{
    std::shared_ptr<RedisRequest> RedisDataComponent::MakeLuaRequest(const std::string &fullName, const std::string &json)
    {
        std::vector<std::string> tempArray;
        if(Helper::String::Split(fullName, ".",tempArray) != 2)
        {
            LOG_ERROR("call lua file error");
            return nullptr;
        }
        const std::string & tab = tempArray[0];
        const std::string & func = tempArray[1];
        auto iter = this->mLuaMap.find(tab);
        if(iter == this->mLuaMap.end())
        {
            LOG_ERROR("not find redis script " << fullName);
            return nullptr;
        }
        const std::string & tag = iter->second;
        return RedisRequest::MakeLua(tag, func, json);
    }

    void RedisDataComponent::OnLoadScript(const std::string &path, const std::string &md5)
    {
        std::string fileName;
        std::vector<std::string> tempArray;
        Helper::String::GetFileName(path, fileName);
        if(Helper::String::Split(fileName, ".",tempArray) != 2)
        {
            LOG_ERROR("add lua script error");
            return;
        }
        const std::string & name = tempArray[0];
        auto iter = this->mLuaMap.find(name);
        if(iter != this->mLuaMap.end())
        {
            this->mLuaMap.erase(iter);
        }
        this->mLuaMap[name] = md5;
    }
}
