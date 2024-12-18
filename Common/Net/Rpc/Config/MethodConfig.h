#pragma once

#include <string>
#include <unordered_set>
namespace acs
{
    class MethodConfig
    {
    public:
		int Timeout;
		bool IsLock; //是否使用分布式锁
		bool IsAsync;
		bool IsRecord;
		std::string Method;
		std::string Headers;
		std::string Server; //所在服务器
		std::string Service; //服务名
	};

	class RpcMethodConfig final : public MethodConfig
	{
	public:
		char Net; //使用的网络
		char Proto;
		char Forward;	//转发策略 0固定转发，1随机转发到某一个机器
		bool IsOpen;
		bool IsAuth;
        bool IsDebug;
		bool IsClient;  //是否能被客户端调用
		bool SendToClient;	//消息是否发送到客户端
		std::string Request;
		std::string Response;
		std::string FullName;
	public:
		std::string NetName;
		std::string ProtoName;
		std::string ForwardName;
	};

	class HttpMethodConfig final : public MethodConfig
	{
	public:
		bool Auth;
		int Limit;
		bool Open;
		int Permission;
		std::string Path;
		std::string Type;
		std::string Desc;
		std::string Token;
		std::string Content;
		std::string Request;
		std::unordered_set<std::string> WhiteList;
	};

}// namespace Sentry