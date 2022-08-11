#include"MainRedisComponent.h"

#include"App/App.h"
#include"Util/StringHelper.h"
#include"Util/DirectoryHelper.h"
#include"Script/ClassProxyHelper.h"
#include"DB/Redis/RedisRpcClient.h"
#include"Script/Extension/Redis/LuaRedis.h"
#include"Component/Scene/NetThreadComponent.h"

namespace Sentry
{
    bool MainRedisComponent::OnInitRedisClient(RedisConfig config)
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


    bool MainRedisComponent::Call(const std::string &name, const std::string &func, Json::Writer &request,
                                  std::shared_ptr<Json::Reader> response)
    {
        SharedRedisClient redisClientContext = this->GetClient(name);
        std::shared_ptr<RedisRequest> redisRequets = this->MakeLuaRequest(func, request.JsonString());
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

	void MainRedisComponent::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{
		luaRegister.BeginRegister<MainRedisComponent>();
		luaRegister.PushExtensionFunction("Run", Lua::Redis::Run);
		luaRegister.PushExtensionFunction("Call", Lua::Redis::Call);
	}
}

namespace Sentry
{
    std::shared_ptr<RedisRequest> MainRedisComponent::MakeLuaRequest(const std::string &fullName, const std::string &json)
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

    void MainRedisComponent::OnLoadScript(const std::string &path, const std::string &md5)
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
