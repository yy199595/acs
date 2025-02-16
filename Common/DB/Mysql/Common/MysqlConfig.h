//
// Created by zmhy0073 on 2022/10/26.
//

#ifndef APP_MYSQLCONFIG_H
#define APP_MYSQLCONFIG_H
#include<vector>
#include"Config/Base/JsonConfig.h"
#include"Core/Singleton/Singleton.h"
#include "Yyjson/Object/JsonObject.h"
namespace mysql
{
	class Config : public json::Object<Config>
    {
    public:
        int ping;
        int count;
		std::string db;
        std::string user;
		std::string passwd;
		std::string address;
		std::string script;
    };
}


#endif //APP_MYSQLCONFIG_H