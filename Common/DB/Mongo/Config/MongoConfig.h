//
// Created by zmhy0073 on 2022/10/26.
//

#ifndef APP_MONGOCONFIG_H
#define APP_MONGOCONFIG_H

#include "DB/Common/Url.h"
#include "DB/Common/Explain.h"

namespace mongo
{
    class Cluster : public json::Object<Cluster>
    {
    public:
		Cluster(): count(1), debug(false) { }
    public:
        int ping = 15;
		int retry = 5; //重试时间(秒)
		bool debug = false;
		int count = 1;
		std::string log;
		std::string auth;
		db::Explain explain;
		int conn_count = 3; //重连次数
		std::vector<std::string> address;
    };

	class Config : public db::Url
	{
	public:
		Config() : db::Url("mongodb") { }
	public:
		int conn_count = 3;
		std::string db;
		std::string user;
		std::string address;
		std::string password;
		std::string mechanism; //验证方式 SCRAM-SHA-1 or SCRAM-SHA-256
	};

	namespace auth
	{
		constexpr const char * SCRAM_SHA1 = "SCRAM-SHA-1";
		constexpr const char * SCRAM_SHA256 = "SCRAM-SHA-256";
	}
}


#endif //APP_MONGOCONFIG_H
