//
// Created by 64658 on 2025/5/30.
//

#ifndef APP_MSCONFIG_H
#define APP_MSCONFIG_H
#include "DB/Common/Url.h"
#include "Yyjson/Object/JsonObject.h"
namespace ms
{
	struct Config : public json::Object<Config>
	{
	public:
		std::string key; //apikey
		std::string address; //地址
	};
}

#endif //APP_MSCONFIG_H
