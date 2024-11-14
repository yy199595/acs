//
// Created by yy on 2023/12/24.
//

#ifndef APP_CONFIG_H
#define APP_CONFIG_H
#include<string>
#include<unordered_set>
namespace http
{
	struct Config
	{
		bool Auth;
		bool RpcDebug;
		std::string Root;
		std::string Index;
		std::string Upload;
		std::string Domain;
        std::unordered_set<std::string> WhiteList;
	};
}
#endif //APP_CONFIG_H
