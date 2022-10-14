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
	public:
		virtual bool IsRpcInterface() const = 0;
		virtual bool IsHttpInterface() const = 0;
	};

	class RpcMethodConfig final : public MethodConfig
	{
	public:
        bool IsAuth;
		std::string Type;
		std::string Request;
		std::string Response;
		std::string FullName;
	public:
		bool IsRpcInterface() const final { return true;}
		bool IsHttpInterface() const final { return false;}
	};

	class HttpMethodConfig final : public MethodConfig
	{
	public:
		std::string Path;
		std::string Type;
		std::string Content;
	public:
		bool IsRpcInterface() const final { return false;}
		bool IsHttpInterface() const final { return true;}
	};

}// namespace Sentry