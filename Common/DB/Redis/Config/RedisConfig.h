//
// Created by zmhy0073 on 2022/10/25.
//

#ifndef APP_REDISCONFIG_H
#define APP_REDISCONFIG_H

#include "DB/Common/Url.h"
#include <Yyjson/Object/JsonObject.h>
namespace redis
{
	struct Cluster final : public json::Object<Cluster>
    {
    public:
		int id = 1;
		int ping = 15;
		int count = 1;
		bool debug = false;
		int conn_count = 3;
		std::string sub;
        std::string script;
		std::vector<std::string> address;
    };

	struct Config : public db::Url
	{
	public:
		Config() : db::Url("redis") { }
	public:
		int db = 0;
		int conn_count = 3;
		std::string address;
		std::string password;
	};
}


#endif //APP_REDISCONFIG_H
