#pragma once
#include<Define/CommonTypeDef.h>
namespace Helper
{
    namespace Net
    {
        extern bool IsIp(const std::string &ip);
        extern bool ParseHttpUrl(const std::string &url, std::string &host, std::string &port, std::string &path);
    };
}