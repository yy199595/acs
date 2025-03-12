//
// Created by zmhy0073 on 2022/10/25.
//

#ifndef APP_REDISCONFIG_H
#define APP_REDISCONFIG_H
#include<unordered_map>
#include <Yyjson/Object/JsonObject.h>
namespace redis
{
	struct Config final : public json::Object<Config>
    {
    public:
		int id;
		int ping;
		int count;
		bool debug;
        std::string name;
        std::string script;
		std::string address;
		std::string password;
    };
}


#endif //APP_REDISCONFIG_H
