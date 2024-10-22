//
// Created by zmhy0073 on 2022/10/25.
//

#ifndef APP_REDISCONFIG_H
#define APP_REDISCONFIG_H
#include<set>
#include<string>
#include<vector>
#include<unordered_map>
namespace redis
{
    struct Config
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
