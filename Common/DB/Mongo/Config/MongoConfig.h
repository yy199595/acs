//
// Created by zmhy0073 on 2022/10/26.
//

#ifndef APP_MONGOCONFIG_H
#define APP_MONGOCONFIG_H
#include<vector>
#include"Config/Base/TextConfig.h"
#include"Core/Singleton/Singleton.h"
namespace mongo
{
    class MongoConfig
    {
    public:
        MongoConfig(): MaxCount(1), Index(0), Ping(0), Debug(false) { }
    public:
        int Ping;
		int Index;
		bool Debug;
		int MaxCount;
        std::string DB;
        std::string User;
		std::string Address;
		std::string Password;
	public:
		bool FromString(const std::string & url);
    };
}


#endif //APP_MONGOCONFIG_H
