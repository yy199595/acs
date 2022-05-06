#pragma once

#include <string>

namespace Sentry
{
    class ProtoConfig
    {
    public:
		bool Auth;
		int Timeout;
        bool IsAsync;
        int MethodId;
		std::string Type;
		std::string CallWay;
		std::string Method;
        std::string Service;
        std::string Request;
        std::string Response;
		std::string FullName;
	};
}// namespace Sentry