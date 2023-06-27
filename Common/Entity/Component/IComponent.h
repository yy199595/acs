#pragma once
#include<string>
#include<memory>
namespace Lua
{
    class ModuleClass;
}
namespace Json
{
    class Writer;
}

namespace Tendo
{
	class Component;
	class IStart
	{
	public:
		virtual void Start() = 0;
	};

	class IComplete
	{
	public:
		virtual void Complete() { }; //启动完毕
	};

	class IDestroy
	{
	public:
		virtual void OnDestroy() = 0;
	};

	class IServer
	{
	public:
		virtual void OnExit(int id) = 0;
		virtual void OnJoin(int id) = 0;
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
		virtual void OnSecondUpdate(int tick) = 0;
	};

	class ILastFrameUpdate
	{
	public:
		virtual void OnLastFrameUpdate() = 0;
	};

	template<typename T>
	class IEvent
	{
	public:
		virtual void OnEvent(const T * message) = 0;
	};

	class ILuaRegister
	{
	public:
		virtual void OnLuaRegister(Lua::ModuleClass& luaRegister, std::string & name) = 0;
	};

	class IHotfix
	{
	public:
		virtual bool OnHotFix() = 0;
	};

	template<typename T1, typename T2>
	class IService
	{
	public:
		virtual int Invoke(const std::string&, const std::shared_ptr<T1> &, std::shared_ptr<T2> &) = 0;
	};

	template<typename T>
	class IRpc
	{
	public:
        virtual ~IRpc() = default;
		virtual void OnTimeout(const std::string & address) { }
		virtual void StartClose(const std::string& address) { };
		virtual void OnConnectSuccessful(const std::string & address) { }
		virtual void OnCloseSocket(const std::string& address, int code) { };
        virtual void OnMessage(std::shared_ptr<T> message) = 0;
		virtual void OnSendFailure(const std::string& address, std::shared_ptr<T> message) { }
    };

    class NodeInfo
    {
    public:
        std::string SrvName;
        std::string UserName;
        std::string PassWord;
    };

    class IServerRecord
    {
    public:
        virtual void OnRecord(Json::Writer & document) = 0;
    };

	class SocketProxy;
	extern std::string GET_FUNC_NAME(const std::string& fullName);
}
