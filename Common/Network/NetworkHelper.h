#pragma once
#include <Define/CommonTypeDef.h>
namespace Sentry
{
	class NetworkHelper
	{
	public:
		static bool IsIp(const std::string & ip);
		static bool ParseHttpUrl(const std::string & url, std::string & host, unsigned short & port, std::string & path);
	};
}