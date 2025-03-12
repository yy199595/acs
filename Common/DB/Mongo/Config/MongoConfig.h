//
// Created by zmhy0073 on 2022/10/26.
//

#ifndef APP_MONGOCONFIG_H
#define APP_MONGOCONFIG_H

#include "DB/Common/Explain.h"

namespace mongo
{
    class MongoConfig : public json::Object<MongoConfig>
    {
    public:
        MongoConfig(): count(1), index(0), ping(0), debug(false) { }
    public:
        int ping;
		int index;
		bool debug;
		int count;
        std::string db;
		std::string log;
		std::string auth;
        std::string user;
		std::string address;
		std::string password;
		std::string mechanism; //验证方式 SCRAM-SHA-1 or SCRAM-SHA-256
		db::Explain explain;
	public:
		bool FromString(const std::string & url);
    };

	namespace auth
	{
		constexpr const char * SCRAM_SHA1 = "SCRAM-SHA-1";
		constexpr const char * SCRAM_SHA256 = "SCRAM-SHA-256";
	}
}


#endif //APP_MONGOCONFIG_H
