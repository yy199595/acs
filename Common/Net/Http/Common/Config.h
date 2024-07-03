//
// Created by yy on 2023/12/24.
//

#ifndef APP_CONFIG_H
#define APP_CONFIG_H
#include<string>
namespace http
{
	struct Config
	{
		bool Auth;
		std::string Root;
		std::string Index;
		std::string Upload;
		std::string Domain;
	};
}
#endif //APP_CONFIG_H
