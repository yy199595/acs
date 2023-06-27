#pragma once

#include <string>

namespace Tendo
{
    class MethodConfig
    {
    public:
		int Timeout;
		bool IsAsync;
		std::string Method;
		std::string Server; //所在服务器
		std::string Service; //服务名
	};

	class RpcMethodConfig final : public MethodConfig
	{
	public:
		int Net; //使用的网络
		int Proto;
		bool IsOpen;
		bool IsClient;
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