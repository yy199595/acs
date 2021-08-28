#pragma once

#include "TcpClientSession.h"
#include <NetWork/NetMessageProxy.h>
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
        LocalActionProxy() {}

        virtual ~LocalActionProxy() {}

    public:
        virtual XCode Invoke(NetMessageProxy *messageData) = 0;
    };
}// namespace Sentry

namespace Sentry
{
    class LocalActionProxy1 : public LocalActionProxy// 无参数 无返回
    {
    public:
        LocalActionProxy1(LocalAction1 action) : mBindAction(action) {}

    public:
        XCode Invoke(NetMessageProxy *messageData) final;

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
        LocalActionProxy2(LocalAction2<T1> action) : mBindAction(action) {}

    public:
		XCode Invoke(NetMessageProxy *messageData) final;

    private:
        LocalAction2<T1> mBindAction;
        ObjectPool<T1> mReqMessagePool;
    };

    template<typename T1>
    inline XCode LocalActionProxy2<T1>::Invoke(NetMessageProxy *messageData)
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
}// namespace Sentry

namespace Sentry
{
    template<typename T1, typename T2>
    class LocalActionProxy3 : public LocalActionProxy//一个参数 一个返回
    {
    public:
        LocalActionProxy3(LocalAction3<T1, T2> action)
            : mBindAction(action) {}

    public:
		XCode Invoke(NetMessageProxy *messageData) override;

    private:
        ObjectPool<T1> mReqMessagePool;
        ObjectPool<T2> mResMessagePool;
        LocalAction3<T1, T2> mBindAction;
    };

    template<typename T1, typename T2>
    inline XCode LocalActionProxy3<T1, T2>::Invoke(NetMessageProxy *messageData)
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
        LocalActionProxy4(LocalAction4<T1> action) : mBindAction(action) {}

		XCode Invoke(NetMessageProxy *messageData) override
        {
            long long userId = messageData->GetUserId();
            T1 *responseData = mResMessagePool.Create();
            XCode code = this->mBindAction(userId, *responseData);

			messageData->SetMessage(*responseData);    
            this->mResMessagePool.Destory(responseData);
            return code;
        }

    private:
        LocalAction4<T1> mBindAction;
        ObjectPool<T1> mResMessagePool;
    };
}// namespace Sentry
