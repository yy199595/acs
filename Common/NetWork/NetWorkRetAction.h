#pragma once

#include<XCode/XCode.h>
#include"NetLuaRetAction.h"
#include<Util/NumberBuilder.h>
#include<Protocol/com.pb.h>
#include<NetWork/NetMessageProxy.h>
#include<NetWork/TcpClientSession.h>

using namespace com;
namespace Sentry
{

    using NetWorkRetAction1 = std::function<void(XCode)>;

    template<typename T>
    using NetWorkRetAction2 = std::function<void(XCode, const T &)>;

    class LocalRetActionProxy
    {
    public:
        LocalRetActionProxy();

        virtual ~LocalRetActionProxy() {}

    public:
        long long GetCreateTime() { return this->mCreateTime; }

    public:
        virtual void Invoke(NetMessageProxy *backData) = 0;

    private:
        long long mCreateTime;
        std::string mFunctionName;
    public:
#ifdef SOEASY_DEBUG
        std::string mService;
        std::string mMethod;
#endif
    };

}

namespace Sentry
{
    class LocalRetActionProxy1 : public LocalRetActionProxy
    {
    public:
        LocalRetActionProxy1(NetWorkRetAction1 action) : mBindAction(action) {}

        ~LocalRetActionProxy1() {}

    public:
        void Invoke(NetMessageProxy *backData) override;

    private:
        NetWorkRetAction1 mBindAction;
    };

    template<typename T>
    class LocalRetActionProxy2 : public LocalRetActionProxy
    {
    public:
        LocalRetActionProxy2(NetWorkRetAction2<T> action) : mBindAction(action) {}

        ~LocalRetActionProxy2() {}

    public:
        void Invoke(NetMessageProxy *backData) override
        {
            mReturnData.Clear();
            XCode code = backData->GetCode();
            if (code != XCode::TimeoutAutoCall)
            {
                if(!this->mReturnData.ParseFromString(backData->GetMsgBody()))
                {
                    this->mBindAction(XCode::ParseMessageError, mReturnData);
                    SayNoDebugError("parse " << typeid(T).name() << " error code:" << code);
                    return;
                }
            }
            this->mBindAction(code, mReturnData);
        }

    private:
        T mReturnData;
        NetWorkRetAction2<T> mBindAction;
    };


    class LocalLuaRetActionProxy : public LocalRetActionProxy
    {
    public:
        LocalLuaRetActionProxy(NetLuaRetAction *action);

        ~LocalLuaRetActionProxy()
        {
            if (mBindLuaAction)
            { delete mBindLuaAction; }
        }

    public:
        void Invoke(NetMessageProxy *backData);

    private:
        NetLuaRetAction *mBindLuaAction;
    };

    class LocalWaitRetActionProxy : public LocalRetActionProxy
    {
    public:
        LocalWaitRetActionProxy(NetLuaWaitAction *action);

        ~LocalWaitRetActionProxy()
        {
            if (mBindLuaAction)
            { delete mBindLuaAction; }
        }

    public:
        void Invoke(NetMessageProxy *backData) override;

    private:
        NetLuaWaitAction *mBindLuaAction;
    };

    class CoroutineComponent;

    class NetWorkWaitCorAction : public LocalRetActionProxy
    {
    public:
        NetWorkWaitCorAction(CoroutineComponent *);

        ~NetWorkWaitCorAction() {}

        static shared_ptr<NetWorkWaitCorAction> Create(CoroutineComponent *);

    public:
        void Invoke(NetMessageProxy *backData) override;

    public:
        XCode GetCode() { return this->mCode; }

        const std::string &GetMsgData() { return this->mMessage; }

    private:
        XCode mCode;
        std::string mMessage;
        long long mCoroutineId;
        CoroutineComponent *mScheduler;
    };
}