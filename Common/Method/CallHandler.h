#pragma once
#include<Script/LuaInclude.h>
#include<XCode/XCode.h>
#include<Util/NumberBuilder.h>
#include<Protocol/com.pb.h>

using namespace com;
namespace GameKeeper
{
    class CallHandler
    {
    public:
		CallHandler();
        virtual ~CallHandler() = default;

    public:
        long long GetCreateTime() { return this->mCreateTime; }

    public:
        virtual void Invoke(const com::DataPacket_Response & backData) = 0;

    private:
        long long mCreateTime;
        std::string mFunctionName;
        class CoroutineComponent * mCorComponent;
    public:
#ifdef SOEASY_DEBUG
        std::string mService;
        std::string mMethod;
#endif
    };

}

namespace GameKeeper
{
    class LuaCallHandler : public CallHandler
    {
    public:
        LuaCallHandler(lua_State *lua, lua_State * cor)
            : mCoroutine(cor), luaEnv(lua) {}
    public:
        void Invoke(const com::DataPacket_Response & backData) override;

    private:
		int ref;
		lua_State *luaEnv;
		lua_State * mCoroutine;
    };

    class CoroutineComponent;

    class CppCallHandler : public CallHandler
    {
    public:
        CppCallHandler();
		~CppCallHandler();
    public:
        void Invoke(const com::DataPacket_Response & backData) override;
    public:
        XCode StartCall();
        XCode StartCall(google::protobuf::Message & message);
    private:
        XCode mCode;
		unsigned int mCoroutineId;
        CoroutineComponent *mScheduler;
        google::protobuf::Message * mMessage;
    };
}