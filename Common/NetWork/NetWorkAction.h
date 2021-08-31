#pragma once

#include "TcpClientSession.h"
#include <NetWork/PacketMapper.h>
#include <Other/ObjectFactory.h>
#include <Pool/ObjectPool.h>
#include <Protocol/com.pb.h>
#include <XCode/XCode.h>

#ifdef SOEASY_DEBUG

#include <google/protobuf/util/json_util.h>

#endif// SOEASY_DEBUG


using namespace com;
namespace Sentry
{
    using LocalAction1 = std::function<XCode(long long)>;

    template<typename T>
    using LocalAction2 = std::function<XCode(long long, const T &)>;

    template<typename T1, typename T2>
    using LocalAction3 = std::function<XCode(long long, const T1 &, T2 &)>;

    template<typename T>
    using LocalAction4 = std::function<XCode(long long, T &)>;

    class LocalActionProxy
    {
    public:
        LocalActionProxy(const std::string name) : mName(name) {}

        virtual ~LocalActionProxy() {}
	public:
		const std::string & GetName() { return this->mName; }
		virtual bool GetRequestType(std::string & requestName) = 0;
		virtual bool GetResponseType(std::string & responseName) = 0;
    public:
        virtual XCode Invoke(PacketMapper *messageData) = 0;
	private:
		const std::string mName;
    };
}// namespace Sentry

namespace Sentry
{
	class LocalActionProxy1 : public LocalActionProxy// 无参数 无返回
	{
	public:
		LocalActionProxy1(const std::string name, LocalAction1 action)
			:LocalActionProxy(name), mBindAction(action) {}

	public:
		XCode Invoke(PacketMapper *messageData) final;
		bool GetRequestType(std::string & requestName) override { return false; };
		bool GetResponseType(std::string & responseName)override { return false; }
	private:
		LocalAction1 mBindAction;
	};
}// namespace Sentry

namespace Sentry
{
    template<typename T1>
    class LocalActionProxy2 : public LocalActionProxy//有参数 无返回
    {
    public:
        LocalActionProxy2(const std::string name, LocalAction2<T1> action)
			:LocalActionProxy(name), mBindAction(action) {}

    public:
		XCode Invoke(PacketMapper *messageData) final;
		bool GetRequestType(std::string & requestName) override;
		bool GetResponseType(std::string & responseName)override { return false; }
    private:
        LocalAction2<T1> mBindAction;
        ObjectPool<T1> mReqMessagePool;
    };

    template<typename T1>
    inline XCode LocalActionProxy2<T1>::Invoke(PacketMapper *messageData)
    {
        T1 *message = mReqMessagePool.Create();
        if (!message->ParseFromString(messageData->GetMsgBody()))
        {
            mReqMessagePool.Destory(message);
            return XCode::ParseMessageError;
        }
		messageData->ClearMessage();
        long long userId = messageData->GetUserId();
        XCode code = this->mBindAction(userId, *message);
        mReqMessagePool.Destory(message);
		return code;
    }
	template<typename T1>
	inline bool LocalActionProxy2<T1>::GetRequestType(std::string & requestName)
	{
		T1 *message = mReqMessagePool.Create();
		requestName = message->GetTypeName();
		mReqMessagePool.Destory(message);
		return true;
	}
}// namespace Sentry

namespace Sentry
{
    template<typename T1, typename T2>
    class LocalActionProxy3 : public LocalActionProxy//一个参数 一个返回
    {
    public:
        LocalActionProxy3(const std::string name, LocalAction3<T1, T2> action)
            : LocalActionProxy(name), mBindAction(action) {}

    public:
		XCode Invoke(PacketMapper *messageData) override;

		bool GetRequestType(std::string & requestName) override;
		bool GetResponseType(std::string & responseName)override;
    private:
        ObjectPool<T1> mReqMessagePool;
        ObjectPool<T2> mResMessagePool;
        LocalAction3<T1, T2> mBindAction;
    };
	template<typename T1, typename T2>
	bool LocalActionProxy3<T1, T2>::GetRequestType(std::string & requestName)
	{
		T1 * request = mReqMessagePool.Create();
		requestName = request->GetTypeName();
		mReqMessagePool.Destory(request);
		return true;
	}

	template<typename T1, typename T2>
	bool LocalActionProxy3<T1, T2>::GetResponseType(std::string & responseName)
	{
		T2 * response = mResMessagePool.Create();
		responseName = response->GetTypeName();
		mResMessagePool.Destory(response);
		return true;
	}

    template<typename T1, typename T2>
    inline XCode LocalActionProxy3<T1, T2>::Invoke(PacketMapper *messageData)
    {
        T1 *request = this->mReqMessagePool.Create();
        if (!request->ParseFromString(messageData->GetMsgBody()))
        {
            mReqMessagePool.Destory(request);
            return XCode::ParseMessageError;
        }
        long long userId = messageData->GetUserId();
        T2 *response = this->mResMessagePool.Create();
        XCode code = this->mBindAction(userId, *request, *response);

		messageData->SetMessage(*response);

        mReqMessagePool.Destory(request);
        mResMessagePool.Destory(response);

		return code;
    }
}// namespace Sentry

namespace Sentry
{
    template<typename T1>
    class LocalActionProxy4 : public LocalActionProxy//无参数 一个返回
    {
    public:
        LocalActionProxy4(const std::string name, LocalAction4<T1> action) 
			:LocalActionProxy(name), mBindAction(action) {}

		XCode Invoke(PacketMapper *messageData) override
        {
            long long userId = messageData->GetUserId();
            T1 *responseData = mResMessagePool.Create();
            XCode code = this->mBindAction(userId, *responseData);

			messageData->SetMessage(*responseData);    
            this->mResMessagePool.Destory(responseData);
            return code;
        }

		bool GetRequestType(std::string & requestName) override
		{
			return false;
		}
		bool GetResponseType(std::string & responseName)override
		{
			T1 *response = mResMessagePool.Create();
			responseName = response->GetTypeName();
			this->mResMessagePool.Destory(response);
			return true;
		}

    private:
        LocalAction4<T1> mBindAction;
        ObjectPool<T1> mResMessagePool;
    };
}// namespace Sentry


namespace Sentry
{
	class ServiceMethod
	{
	public:
		ServiceMethod(const std::string name) : mName(name) { }
		virtual XCode Invoke(PacketMapper *messageData) = 0;
		const std::string & GetName() { return this->mName; }
	private:
		std::string mName;
	};
	template<typename T>
	class ServiceMethod1 : public ServiceMethod
	{
	public:
		typedef XCode(T::*ServerFunc)(long long);
		ServiceMethod1(const std::string name, T * o, ServerFunc func)
			: ServiceMethod(name) ,_o(o), _func(func) { }
	public:
		XCode Invoke(PacketMapper *messageData) override
		{
			return (_o->*_func)(messageData->GetUserId());
		}
	private:
		T * _o;
		ServerFunc _func;
	};
	template<typename T, typename T1>
	class ServiceMethod2 : public ServiceMethod
	{
	public:
		typedef XCode(T::*ServerFunc)(long long, const T1 &);
		ServiceMethod2(const std::string name, T * o, ServerFunc func)
			:ServiceMethod(name), _o(o), _func(func) { }
	public:
		XCode Invoke(PacketMapper *messageData) override
		{
			T1 * request = mReqMessagePool.Create();
			const std::string & data = messageData->GetMsgBody();
			if (!request->ParseFromString(data))
			{
				mReqMessagePool.Destory(request);
				return XCode::ParseMessageError;
			}
			XCode code = (_o->*_func)(messageData->GetUserId(), *request);
			mReqMessagePool.Destory(request);
			return code;
		}
	private:
		T * _o;
		ServerFunc _func;
		ObjectPool<T1> mReqMessagePool;
	};

	template<typename T, typename T1, typename T2>
	class ServiceMethod3 : public ServiceMethod
	{
	public:
		typedef XCode(T::*ServerFunc)(long long, const T1 &, T2 &);
		ServiceMethod3(const std::string name, T * o, ServerFunc func)
			: ServiceMethod(name),_o(o), _func(func) { }
	public:
		XCode Invoke(PacketMapper *messageData) override
		{
			T1 * request = mReqMessagePool.Create();
			const std::string & data = messageData->GetMsgBody();
			if (!request->ParseFromString(data))
			{
				mReqMessagePool.Destory(request);
				return XCode::ParseMessageError;
			}
			T2 * response = mResMessagePool.Create();
			XCode code = (_o->*_func)(messageData->GetUserId(), *request, *response);
			if (code == XCode::Successful)
			{
				messageData->SetMessage(*response);			
			}
			mReqMessagePool.Destory(request);
			mResMessagePool.Destory(response);
			return code;
		}
	private:
		T * _o;
		ServerFunc _func;
		std::string mBuffer;
		ObjectPool<T1> mReqMessagePool;
		ObjectPool<T2> mResMessagePool;
	};

	template<typename T, typename T1>
	class ServiceMethod4 : public ServiceMethod
	{
	public:
		typedef XCode(T::*ServerFunc)(long long, T1 &);
		ServiceMethod4(const std::string name, T * o, ServerFunc func)
			:ServiceMethod(name), _o(o), _func(func) { }
	public:
		XCode Invoke(PacketMapper *messageData) override
		{
			T1 * response = mResMessagePool.Create();			
			XCode code = (_o->*_func)(messageData->GetUserId(), *response);
			mResMessagePool.Destory(response);
			return code;
		}
	private:
		T * _o;
		ServerFunc _func;
		ObjectPool<T1> mResMessagePool;
	};
}