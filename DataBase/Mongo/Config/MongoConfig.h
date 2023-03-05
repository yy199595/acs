//
// Created by zmhy0073 on 2022/10/26.
//

#ifndef APP_MONGOCONFIG_H
#define APP_MONGOCONFIG_H
#include<vector>
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
        std::string DB;
        std::string User;
        std::string Password;
        std::vector<Net::Address> Address;
    };
}


#endif //APP_MONGOCONFIG_H
