#pragma once
#include<Thread/TaskThread.h>
#include<Message/com.pb.h>
#include"Message/c2s.pb.h"
#include"XCode/XCode.h"
#include"Json/JsonReader.h"
#include"Json/JsonWriter.h"
#include"Script/ClassProxyHelper.h"
namespace Sentry
{
	class Component;
	class IStart
	{
	public:
		virtual bool OnStart() = 0;
	};

	class IComplete
	{
	public:
		virtual void OnComplete() { };
		virtual void OnAllServiceStart() { }
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

	class IServiceBase
	{
	public:
		virtual bool StartService() = 0;
		virtual bool CloseService() = 0;
		virtual bool IsStartService() = 0;
		virtual bool IsStartComplete() = 0;
		virtual bool LoadConfig(const rapidjson::Value& json) = 0;
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
		virtual void OnAddService(class Component* component) = 0;
		virtual void OnDelService(class Component* component) = 0;
	};

	template<typename T1, typename T2>
	class IRpc
	{
	public:
		virtual void OnRequest(std::shared_ptr<T1> t1) = 0;
		virtual void OnResponse(std::shared_ptr<T2> t2) = 0;
		virtual void StartClose(const std::string& address) = 0;
		virtual void OnCloseSocket(const std::string& address, XCode code) = 0;
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
	class ISocketListen
	{
	public:
		virtual void OnListen(std::shared_ptr<SocketProxy> socket) = 0;
	};
}