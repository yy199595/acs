//
// Created by zmhy0073 on 2022/10/25.
//

#include"RedisConfig.h"
#include"Log/CommonLogDef.h"
#include"rapidjson/document.h"
namespace Sentry
{
    bool RedisConfig::OnLoadText(const char *str, size_t length)
    {
        rapidjson::Document document;
        if(document.Parse(str, length).HasParseError())
        {
            return false;
        }
        for(auto iter = document.MemberBegin(); iter != document.MemberEnd(); iter++)
        {
            const std::string name(iter->name.GetString());
            const rapidjson::Value & jsonData = iter->value;
            LOG_CHECK_RET_FALSE(jsonData.HasMember("ip"));
            LOG_CHECK_RET_FALSE(jsonData.HasMember("port"));
            LOG_CHECK_RET_FALSE(jsonData.HasMember("index"));
            LOG_CHECK_RET_FALSE(jsonData.HasMember("count"));

            RedisClientConfig redisConfig;
            redisConfig.Name = name;
            redisConfig.FreeClient = 30;
            redisConfig.Ip = jsonData["ip"].GetString();
            redisConfig.Port = jsonData["port"].GetInt();
            redisConfig.Index = jsonData["index"].GetInt();
            redisConfig.Count = jsonData["count"].GetInt();
            if (jsonData.HasMember("free"))
            {
                redisConfig.FreeClient = jsonData["free"].GetInt();
            }
            if (jsonData.HasMember("passwd"))
            {
                redisConfig.Password = jsonData["passwd"].GetString();
            }
            if (jsonData.HasMember("scripts") && jsonData["scripts"].IsObject())
            {
                const rapidjson::Value & value = jsonData["scripts"];
                for(auto iter = value.MemberBegin(); iter != value.MemberEnd(); iter++)
                {
                    const std::string key(iter->name.GetString());
                    const std::string value(iter->value.GetString());
                    redisConfig.LuaFiles.emplace(key, this->WorkPath() + value);
                }
            }
            redisConfig.Address = fmt::format("{0}:{1}", redisConfig.Ip, redisConfig.Port);
            this->mConfigs.emplace(name, redisConfig);
        }
        return true;
    }

    bool RedisConfig::Get(const std::string &name, RedisClientConfig &config) const
    {
        auto iter = this->mConfigs.find(name);
        if(iter == this->mConfigs.end())
        {
            return false;
        }
        config = iter->second;
        return true;
    }

    bool RedisConfig::Has(const std::string &name) const
    {
        auto iter = this->mConfigs.find(name);
        return iter != this->mConfigs.end();
    }

    bool RedisConfig::Get(std::vector<RedisClientConfig> &configs) const
    {
        auto iter = this->mConfigs.begin();
        for(; iter != this->mConfigs.end(); iter++)
        {
            configs.emplace_back(iter->second);
        }
        return !configs.empty();
    }
}