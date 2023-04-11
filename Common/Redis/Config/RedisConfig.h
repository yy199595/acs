//
// Created by zmhy0073 on 2022/10/25.
//

#ifndef APP_REDISCONFIG_H
#define APP_REDISCONFIG_H
#include<set>
#include<string>
#include<vector>
#include<unordered_map>
#include"Server/Config/TextConfig.h"
namespace Tendo
{
    struct RedisClientConfig
    {
    public:
        int Count;
        int Index;
        int FreeClient;
        std::string Name;
        std::string Script;
        std::string Password;
        std::vector<Net::Address> Address;
    };


    class RedisConfig : public TextConfig
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
