//
// Created by yy on 2023/12/24.
//

#ifndef APP_CONFIG_H
#define APP_CONFIG_H
#include "Yyjson/Object/JsonObject.h"
namespace http
{
	struct Config : public json::Object<Config>
	{
		bool auth;
		std::string root;
		std::string index;
		std::string upload;
		std::string domain;
        std::vector<std::string> whiteList;
		std::unordered_map<std::string, std::string> header;
	};
}
#endif //APP_CONFIG_H
