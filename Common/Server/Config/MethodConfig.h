#pragma once

#include <string>

namespace Sentry
{
    class MethodConfig
    {
    public:
		int Timeout;
		bool IsAsync;
		std::string Method;
        std::string Service;
	};

	class RpcMethodConfig final : public MethodConfig
	{
	public:
		std::string Type;
		std::string Request;
		std::string Response;
		std::string FullName;
	};

	class HttpMethodConfig final : public MethodConfig
	{
	public:
		std::string Path;
		std::string Type;
		std::string Content;
	};

}// namespace Sentry