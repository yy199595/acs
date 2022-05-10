#pragma once

#include <string>

namespace Sentry
{
    class InterfaceConfig
    {
    public:
		int Timeout;
		std::string Method;
        std::string Service;
	public:
		virtual bool IsRpcInterface() = 0;
		virtual bool IsHttpInterface() = 0;
	};

	class RpcInterfaceConfig final : public InterfaceConfig
	{
	public:
		bool IsAsync;
		int InterfaceId;
		std::string Type;
		std::string CallWay;
		std::string Request;
		std::string Response;
		std::string FullName;
	public:
		bool IsRpcInterface() final { return true;}
		bool IsHttpInterface() final { return false;}
	};

	class HttpInterfaceConfig final : public InterfaceConfig
	{
	public:
		std::string Path;
		std::string Type;
		std::string Content;
	public:
		bool IsRpcInterface() final { return false;}
		bool IsHttpInterface() final { return true;}
	};

}// namespace Sentry