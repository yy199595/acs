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
		int Id;
		int Ping;
		int Count;
        int Index;
		bool Debug;
        std::string Name;
        std::string Script;
		std::string Address;
		std::string Password;
    };
}


#endif //APP_REDISCONFIG_H
