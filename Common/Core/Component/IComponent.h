#pragma once
#include<string>
#include<memory>
namespace Lua
{
    class ClassProxyHelper;
}
namespace Json
{
    class Writer;
}

namespace Sentry
{
	class Component;
	class IStart
	{
	public:
		virtual bool Start() = 0;
	};

	class IComplete
	{
	public:
		virtual void OnLocalComplete() { }; //本机所有服务启动完毕
		virtual void OnClusterComplete() { } //集群内所有服务启动完毕
	};

	class IFrameUpdate
	{
	public:
		virtual void OnFrameUpdate(float t) = 0;
	};

	class ISystemUpdate
	{
	public:
		virtual void OnSystemUpdate() = 0;
	};

	class ISecondUpdate
	{
	public:
		virtual void OnSecondUpdate(const int tick) = 0;
	};

	class ILastFrameUpdate
	{
	public:
		virtual void OnLastFrameUpdate() = 0;
	};

	class ILuaRegister
	{
	public:
		virtual void OnLuaRegister(Lua::ClassProxyHelper& luaRegister) = 0;
	};

	class IHotfix
	{
	public:
		virtual void OnHotFix() = 0;
	};

	class ILoadConfig
	{
	public:
		virtual bool OnLoadConfig() = 0;
	};

	class IZeroRefresh
	{
	public:
		virtual void OnZeroRefresh() = 0;
		virtual void GetRefreshTime(int& hour, int& min)
		{
			hour = 0;
			min = 0;
		}
	};

    class IServiceUnitSystem
    {
    public:
        virtual void OnLogin(long long userId) = 0;
        virtual void OnLogout(long long userId) = 0;
    };

	class IServiceBase
	{
	public:
        virtual void Init() { }
		virtual bool Start() = 0;
		virtual bool Close() = 0;
		virtual bool LoadFromLua() = 0; //在热更新的时候调用
        virtual void OnCloseComplete() { }
		virtual bool IsStartService() = 0;
        virtual void WaitAllMessageComplete() { };
        virtual int GetWaitMessageCount() const { return 0; }
	};

	template<typename T1, typename T2>
	class IService : public IServiceBase
	{
	public:
		virtual int Invoke(const std::string&, std::shared_ptr<T1>, std::shared_ptr<T2>) = 0;
	};

	template<typename T>
	class IRpc
	{
	public:
        virtual ~IRpc() { }
		virtual void StartClose(const std::string& address) { };
		virtual void OnConnectSuccessful(const std::string & address) { }
		virtual void OnCloseSocket(const std::string& address, int code) { };
        virtual void OnMessage(std::shared_ptr<T> message) = 0;
		virtual void OnSendFailure(const std::string& address, std::shared_ptr<T> message) { }
    };

    class ServiceNodeInfo
    {
    public:
        std::string SrvName;
        std::string UserName;
        std::string PassWord;
        std::string RpcAddress;
        std::string HttpAddress;
		std::string LocalAddress;
    };

	template<typename T1, typename T2>
	class IClientRpc
	{
	public:
		virtual void OnConnect(const std::string& address) { }
		virtual int OnRequest(std::shared_ptr<T1> request) = 0;
		virtual int OnResponse(const std::string&, const std::shared_ptr<T2> response) = 0;
	};

    class IServerRecord
    {
    public:
        virtual void OnRecord(Json::Writer & document) = 0;
    };

	class SocketProxy;
}