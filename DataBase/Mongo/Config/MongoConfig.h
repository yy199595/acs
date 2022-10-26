//
// Created by zmhy0073 on 2022/10/26.
//

#ifndef APP_MONGOCONFIG_H
#define APP_MONGOCONFIG_H
#include"Config/TextConfig.h"
#include"Singleton/Singleton.h"
namespace Sentry
{
    class MongoConfig : public TextConfig, public ConstSingleton<MongoConfig>
    {
    public:
        MongoConfig() : TextConfig("MongoConfig") { }

    public:
       bool OnLoadText(const char *str, size_t length) final;
        bool OnReloadText(const char *str, size_t length) final;
    public:
        int MaxCount;
        std::string Ip;
        std::string DB;
        unsigned int Port;
        std::string User;
        std::string Address;
        std::string Password;
    };
}


#endif //APP_MONGOCONFIG_H
