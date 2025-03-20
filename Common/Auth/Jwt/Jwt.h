//
// Created by yy on 2024/1/1.
//

#ifndef APP_JWT_H
#define APP_JWT_H

#include<string>
namespace jwt
{
	extern std::string Create(const std::string& payload, const std::string& secret);
	extern bool Verify(const std::string& jwt, const std::string& secret, std::string & payload);
}

#endif //APP_JWT_H
