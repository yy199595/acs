//
// Created by zmhy0073 on 2022/10/26.
//

#ifndef APP_MYSQLCONFIG_H
#define APP_MYSQLCONFIG_H
#include<vector>
#include"Config/Base/JsonConfig.h"
#include"Core/Singleton/Singleton.h"

namespace mysql
{
    class MysqlConfig
    {
    public:
        int Ping;
        int MaxCount;
		std::string DB;
        std::string User;
		std::string Address;
        std::string Password;
		std::vector<std::string> Tables;
    };
}


#endif //APP_MYSQLCONFIG_H