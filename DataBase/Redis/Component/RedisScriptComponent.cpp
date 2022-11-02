//
// Created by zmhy0073 on 2022/11/2.
//

#include"RedisScriptComponent.h"
#include"String/StringHelper.h"
#include"RedisComponent.h"
namespace Sentry
{
    bool RedisScriptComponent::LateAwake()
    {
        this->mComponent = this->GetComponent<RedisComponent>();
        return true;
    }

    bool RedisScriptComponent::Start()
    {
        return true;
    }

    void RedisScriptComponent::OnLoadScript(const std::string &path, const std::string &md5)
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

    std::shared_ptr<RedisRequest> RedisScriptComponent::MakeLuaRequest(const std::string &fullName, const std::string &json)
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

    std::unique_ptr<std::string> RedisScriptComponent::Call(const std::string &name, const std::string &func, const std::string &json)
    {
        TcpRedisClient * redisClientContext = this->mComponent->GetClient(name);
        std::shared_ptr<RedisRequest> redisRequets = this->MakeLuaRequest(func, json);
        if(redisRequets == nullptr)
        {
            return nullptr;
        }
        std::shared_ptr<RedisResponse> redisResponse = this->mComponent->Run(redisClientContext, redisRequets);
        if (redisResponse == nullptr || redisResponse->HasError())
        {
            return nullptr;
        }
#ifdef __DEBUG__
        LOG_DEBUG("call lua " << func << " response json = " << redisResponse->GetString());
#endif
        const std::string & str = redisResponse->GetString();
        return std::make_unique<std::string>(std::move(str));
    }

}