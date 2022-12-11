//
// Created by zmhy0073 on 2022/11/2.
//

#include"RedisScriptComponent.h"
#include"String/StringHelper.h"
#include"RedisComponent.h"
#include"File/FileHelper.h"
#include"System/System.h"
namespace Sentry
{
    bool RedisScriptComponent::LateAwake()
    {
        this->mComponent = this->GetComponent<RedisComponent>();
        return true;
    }

    bool RedisScriptComponent::Start()
    {
        std::vector<RedisClientConfig> configs;
        if (RedisConfig::Inst()->Get(configs))
        {
            for (const RedisClientConfig &config: configs)
            {
                if (config.LuaFiles.empty())
                {
                    continue;
                }
                TcpRedisClient *redisClient = this->mComponent->GetClient(config.Name);
                for (auto iter = config.LuaFiles.begin(); iter != config.LuaFiles.end(); iter++)
                {
                    std::string content;
                    const std::string & name = iter->first;
                    const std::string & path = iter->second;
                    if(!Helper::File::ReadTxtFile(path, content))
                    {
                        LOG_ERROR("load lua [" << path << "] error");
                        return false;
                    }
                    std::shared_ptr<RedisResponse> response =
                        this->mComponent->Run(redisClient, "SCRIPT", "LOAD", content);
                    if(response == nullptr || response->HasError())
                    {
                        return false;
                    }
                    LOG_CHECK_RET_FALSE(this->OnLoadScript(name, response->GetString()));
                }
            }
        }
        return true;
    }

    bool RedisScriptComponent::OnLoadScript(const std::string &name, const std::string &md5)
    {
        auto iter = this->mLuaMap.find(name);
        if(iter != this->mLuaMap.end())
        {
            return false;
        }
        this->mLuaMap.emplace(name, md5);
        //CONSOLE_LOG_INFO("add lua file " << name << ":" << md5);
        return true;
    }

    std::shared_ptr<RedisRequest> RedisScriptComponent::MakeLuaRequest(const std::string &fullName, const std::string &json)
    {
        size_t pos = fullName.find('.');
        if(pos == std::string::npos)
        {
            return nullptr;
        }
        const std::string tab = fullName.substr(0, pos);
        const std::string func = fullName.substr(pos + 1);
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