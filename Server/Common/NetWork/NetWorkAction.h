#pragma once
#include<XCode/XCode.h>
#include"TcpClientSession.h"
#include<Protocol/com.pb.h>
#include<Other/ObjectFactory.h>
#include<Pool/ObjectPool.h>
#include<NetWork/NetMessageProxy.h>
#ifdef SOEASY_DEBUG
#include<google/protobuf/util/json_util.h>
#endif // SOEASY_DEBUG


using namespace com;
namespace Sentry
{
	using LocalAction1 = std::function<XCode(long long)>;

	template <typename T>
	using LocalAction2 = std::function<XCode(long long, const T &)>;

	template <typename T1, typename T2>
	using LocalAction3 = std::function<XCode(long long, const T1 &, T2 &)>;

	template <typename T>
	using LocalAction4 = std::function<XCode(long long, T &)>;

	class LocalActionProxy
	{
	public:
		LocalActionProxy() {}
		virtual ~LocalActionProxy() {}
	public:
		virtual XCode Invoke(long long id, Message * request, Message * response) = 0;
	};
}

namespace Sentry
{
	class LocalActionProxy1 : public LocalActionProxy // 无参数 无返回
	{
	public:
		LocalActionProxy1(LocalAction1 action) : mBindAction(action) {}

	public:
		XCode Invoke(long long id, Message * request, Message * response) final;

	private:
		LocalAction1 mBindAction;
	};
}

namespace Sentry
{
	template <typename T1>
	class LocalActionProxy2 : public LocalActionProxy //有参数 无返回
	{
	public:
		LocalActionProxy2(LocalAction2<T1> action) : mBindAction(action) {}

	public:
		XCode Invoke(long long id, Message * request, Message * response) final;
	private:
		LocalAction2<T1> mBindAction;
	};

	template <typename T1>
	inline XCode LocalActionProxy2<T1>::Invoke(long long id, Message * request, Message * response)
	{
		T1 * message = static_cast<T1*>(request);
		return this->mBindAction(id, *message);
	}
}

namespace Sentry
{
	template <typename T1, typename T2>
	class LocalActionProxy3 : public LocalActionProxy //一个参数 一个返回
	{
	public:
		LocalActionProxy3(LocalAction3<T1, T2> action)
			:  mBindAction(action) { }

	public:
		XCode Invoke(long long id, Message * request, Message * response) override;
	private:
		LocalAction3<T1, T2> mBindAction;
	};

	template <typename T1, typename T2>
	inline XCode LocalActionProxy3<T1, T2>::Invoke(long long id, Message * request, Message * response)
	{
		T1 * requestData = static_cast<T1*>(request);
		T2 * responseData = static_cast<T2*>(response);
		return this->mBindAction(id, *requestData, *responseData);
	}
}

namespace Sentry
{
	template<typename T1>
	class LocalActionProxy4 : public LocalActionProxy //无参数 一个返回
	{
	public:
		LocalActionProxy4(LocalAction4<T1> action) :mBindAction(action) {}

		XCode Invoke(long long id, Message * request, Message * response) override
		{
			T1 * responseData = static_cast<T1*>(response);
			return this->mBindAction(id, *responseData);
		}
	private:
		LocalAction4<T1> mBindAction;
	};
}
