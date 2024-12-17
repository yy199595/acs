#pragma once
#include<string>
#include<memory>
#include<limits>
#include"Yyjson/Document/Document.h"
namespace Lua
{
    class ModuleClass;
}

namespace acs
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

	class IAppStop
	{
	public:
		virtual void OnAppStop() = 0;
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
		virtual void OnFrameUpdate(long long) = 0;
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

	class ISystemNewDay
	{
	public:
		virtual void OnNewDay() = 0;
	};

	class ILastFrameUpdate
	{
	public:
		virtual void OnLastFrameUpdate(long long) = 0;
	};

	class ILuaRegister
	{
	public:
		virtual void OnLuaRegister(Lua::ModuleClass& luaRegister) = 0;
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
		virtual int Invoke(const std::string&, const T1 &, T2 &) = 0;
	};

	template<typename C, typename T1, typename T2>
	class IRequest
	{
	public:
		virtual int OnRequest(const C & c, const T1 & t1) { return 0; }
		virtual void OnRequestDone(const C & c, const T1 & t1, const T2 & t2) { }
	};

	template<typename T1, typename T2>
	class IRpc
	{
	public:
        virtual ~IRpc() = default;
		virtual void OnTimeout(int id) { };
		virtual void StartClose(int id) { };
		virtual void StartClose(int id, int) { };
		virtual void OnConnectOK(int id) { }
		virtual void OnClientError(int id, int code) { };
		virtual void OnSendFailure(int id, T1 * message) { }
		virtual void OnMessage(T1* request, T2* response) { };
		virtual void OnReadHead(T1* request, T2 * response) { }
		virtual void OnMessage(int, T1* request, T2* response) { };
	};

	class ILogin
	{
	public:
		virtual void OnLogin(long long player) = 0;
		virtual void OnLogout(long long player) = 0;
	};

	class IOnRegister
	{
	public:
		virtual void OnRegister(long long userId) = 0;
	};

    class IServerRecord
    {
    public:
        virtual void OnRecord(json::w::Document & document) = 0;
    };

	template<typename T1, typename T2 = T1>
	class ICustomDisMessage
	{
	public:
		virtual bool OnMessage(T1 * t1, T2 * t) { return false; }
	};

	class SocketProxy;
	extern std::string GET_FUNC_NAME(const std::string& fullName);
}

namespace math
{
	template<typename T, size_t limit = 10>
	class NumberPool
	{
	public:
		NumberPool() : mIndex(0) { }
		explicit NumberPool(T start) : mIndex(start) { }
	public:
		inline T BuildNumber()
		{
			++this->mIndex;
			if(this->mIndex >= this->MaxNum)
			{
				this->mIndex = 1;
			}
			return this->mIndex;
		}
		inline T CurrentNumber() const { return this->mIndex; }
	private:
		T mIndex;
		T MaxNum = std::numeric_limits<T>::max() - limit;
	};
}
