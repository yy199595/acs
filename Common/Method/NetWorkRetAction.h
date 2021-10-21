#pragma once
#include<Script/LuaInclude.h>
#include<XCode/XCode.h>
#include<Util/NumberBuilder.h>
#include<Protocol/com.pb.h>

using namespace com;
namespace Sentry
{
    class LocalRetActionProxy
    {
    public:
		LocalRetActionProxy();

        virtual ~LocalRetActionProxy() {}

    public:
        long long GetCreateTime() { return this->mCreateTime; }

    public:
        virtual void Invoke(const com::DataPacket_Response & backData) = 0;

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
    class LocalWaitRetActionProxy : public LocalRetActionProxy
    {
    public:
        LocalWaitRetActionProxy(lua_State *lua, lua_State * cor) : mCoroutine(cor), luaEnv(lua) {}
    public:
        void Invoke(const com::DataPacket_Response & backData) override;

    private:
		int ref;
		lua_State *luaEnv;
		lua_State * mCoroutine;
    };

    class CoroutineComponent;

    class NetWorkWaitCorAction : public LocalRetActionProxy
    {
    public:
        NetWorkWaitCorAction(CoroutineComponent *);

		~NetWorkWaitCorAction();

        static shared_ptr<NetWorkWaitCorAction> Create();

    public:
        void Invoke(const com::DataPacket_Response & backData) override;
    public:
        XCode GetCode() { return this->mCode; }

        const std::string &GetMsgData() { return this->mMessage; }

    private:
        XCode mCode;
        std::string mMessage;
		unsigned int mCoroutineId;
        CoroutineComponent *mScheduler;
    };
}