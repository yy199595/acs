//
// Created by zmhy0073 on 2022/10/26.
//
#ifdef __ENABLE_MYSQL__
#ifndef APP_MYSQLCONFIG_H
#define APP_MYSQLCONFIG_H
#include<vector>
#include"Server/Config/TextConfig.h"
#include"Core/Singleton/Singleton.h"
namespace Sentry
{
    class MysqlConfig : public TextConfig
    {
    public:
        MysqlConfig() : TextConfig("MysqlConfig"), MaxCount(0) { }
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

#endif