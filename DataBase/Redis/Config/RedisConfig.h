//
// Created by zmhy0073 on 2022/10/25.
//

#ifndef APP_REDISCONFIG_H
#define APP_REDISCONFIG_H
#include<set>
#include<string>
#include<vector>
#include<unordered_map>
#include"Config/TextConfig.h"
#include<Singleton/Singleton.h>
namespace Sentry
{
    struct RedisClientConfig
    {
    public:
        int Count;
        int Index;
        int FreeClient;
        std::string Ip;
        std::string Name;
        unsigned short Port;
        std::string Address;
        std::string Password;
        std::unordered_map<std::string, std::string> LuaFiles;
    };


    class RedisConfig : public TextConfig, public ConstSingleton<RedisConfig>
    {
    public:
        RedisConfig() : TextConfig("RedisConfig") { }
    public:
		bool OnLoadText(const char *str, size_t length) final;
		const RedisClientConfig & Config() const { return this->mConfig; }
		bool OnReloadText(const char *str, size_t length) final { return true; }
    private:
		RedisClientConfig mConfig;
    };
}


#endif //APP_REDISCONFIG_H
