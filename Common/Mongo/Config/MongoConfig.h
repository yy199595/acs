//
// Created by zmhy0073 on 2022/10/26.
//

#ifndef APP_MONGOCONFIG_H
#define APP_MONGOCONFIG_H
#include<vector>
#include"Server/Config/TextConfig.h"
#include"Core/Singleton/Singleton.h"
namespace Mongo
{
    class MongoConfig : public Tendo::TextConfig
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
        Net::Address Address;
    };
}


#endif //APP_MONGOCONFIG_H
