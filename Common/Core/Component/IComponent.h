#pragma once
#include"Message/com.pb.h"
#include"Message/c2s.pb.h"
#include"XCode/XCode.h"
#include"Json/JsonReader.h"
#include"Json/JsonWriter.h"
#include"Lua/ClassProxyHelper.h"
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
		virtual bool Start() = 0;
		virtual bool Close() = 0;
        virtual void OnCloseComplete() { }
		virtual bool IsStartService() = 0;
		virtual bool IsStartComplete() = 0;
        virtual void WaitAllMessageComplete() { };
        virtual int GetWaitMessageCount() const { return 0; }
	};

	template<typename T1, typename T2>
	class IService : public IServiceBase
	{
	public:
		virtual XCode Invoke(const std::string&, std::shared_ptr<T1>, std::shared_ptr<T2>) = 0;
	};

	class IServiceChange
	{
	public:
		virtual void OnAddService(const std::string & name) = 0;
		virtual void OnDelService(const std::string & name) = 0;
	};

	template<typename T>
	class IRpc
	{
	public:
        virtual ~IRpc() { }
		virtual void StartClose(const std::string& address) = 0;
		virtual void OnCloseSocket(const std::string& address, XCode code) = 0;
        virtual void OnMessage(const std::string & address, std::shared_ptr<T> message) = 0;
    };

    class ServiceNodeInfo
    {
    public:
        std::string SrvName;
        std::string UserName;
        std::string PassWord;
        std::string LocationRpc;
        std::string LocationHttp;
    };

	template<typename T1, typename T2>
	class IClientRpc
	{
	public:
		virtual void OnConnect(const std::string& address) { }
		virtual XCode OnRequest(std::shared_ptr<T1> request) = 0;
		virtual XCode OnResponse(const std::string&, const std::shared_ptr<T2> response) = 0;
	};

	class SocketProxy;
}