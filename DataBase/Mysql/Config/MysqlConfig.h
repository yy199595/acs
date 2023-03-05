//
// Created by zmhy0073 on 2022/10/26.
//

#ifndef APP_MYSQLCONFIG_H
#define APP_MYSQLCONFIG_H
#include<vector>
#include"Config/TextConfig.h"
#include"Singleton/Singleton.h"
namespace Sentry
{
    class MysqlConfig : public TextConfig, public ConstSingleton<MysqlConfig>
    {
    public:
        MysqlConfig() : TextConfig("MysqlConfig") { }
    protected:
       bool OnLoadText(const char *str, size_t length) final;
        bool OnReloadText(const char *str, size_t length) final;
    public:
        int MaxCount;
        std::string User;
        std::string Password;
        std::vector<Net::Address> Address;
    };
}


#endif //APP_MYSQLCONFIG_H
