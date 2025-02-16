#pragma once
#include<string>
#include<memory>
#include<limits>
#include"Yyjson/Document/Document.h"
namespace Lua
{
    class CCModule;
}

namespace acs
{
	class Component;
	class IStart
	{
	public:
		virtual void OnStart() = 0;
	};

	class IComplete
	{
	public:
		virtual void OnComplete() { }; //启动完毕
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

	//每帧调用
	class IFrameUpdate
	{
	public:
		virtual void OnFrameUpdate(long long) noexcept = 0;
	};

	//每次循环调用
	class ISystemUpdate
	{
	public:
		virtual void OnSystemUpdate() noexcept = 0;
	};

	//每秒调用
	class ISecondUpdate
	{
	public:
		virtual void OnSecondUpdate(int tick) noexcept = 0;
	};

	//新一天调用
	class ISystemNewDay
	{
	public:
		virtual void OnNewDay() = 0;
	};

	//下一帧调用
	class ILastFrameUpdate
	{
	public:
		virtual void OnLastFrameUpdate(long long) noexcept = 0;
	};

	// 热重载调用
	class IHotfix
	{
	public:
		virtual bool OnHotFix() = 0;
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
		virtual void StartClose(int id) { };
		virtual void StartClose(int id, int) { };
		virtual void OnConnectOK(int id) { }
		virtual void OnClientError(int id, int code) { };
		virtual void OnSendFailure(int id, T1 * message) { }
		virtual void OnMessage(T1* request, T2* response) noexcept { };
		virtual void OnReadHead(T1* request, T2 * response) noexcept { }
		virtual void OnMessage(int, T1* request, T2* response) noexcept { };
	};

	class ILogin
	{
	public:
		virtual void OnLogin(long long player) noexcept = 0;
		virtual void OnLogout(long long player) noexcept = 0;
	};

    class IServerRecord
    {
    public:
        virtual void OnRecord(json::w::Document & document) = 0;
    };

	extern std::string GET_FUNC_NAME(const std::string& fullName);
}

namespace math
{
	template<typename T, size_t limit = 10>
	class NumberPool
	{
	public:
		NumberPool() : mIndex(0), mCount(0) { }
		explicit NumberPool(T start) : mIndex(start), mCount(0) { }
	public:
		inline T BuildNumber() noexcept
		{
			++this->mIndex;
			if(this->mIndex >= this->MaxNum)
			{
				++ this->mCount;
				this->mIndex = 1;
			}
			return this->mIndex;
		}
		inline T CurrentNumber() const { return this->mIndex; }
	private:
		T mIndex;
		int mCount;
		T MaxNum = std::numeric_limits<T>::max() - limit;
	};
}
