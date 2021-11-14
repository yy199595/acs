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
		CallHandler(int method);
		virtual ~CallHandler();

    public:
		int GetMethodId() const { return this->mMethodId; }
        long long GetCreateTime() const { return this->mCreateTime; }	
    public:
        virtual void Invoke(const com::Rpc_Response & backData) = 0;
    private:
		int mMethodId;
        long long mCreateTime;
        class CoroutineComponent * mCorComponent;
    };

}

namespace GameKeeper
{
    class LuaCallHandler : public CallHandler
    {
    public:
        LuaCallHandler(int method, lua_State *lua, lua_State * cor)
            :CallHandler(method), mCoroutine(cor), luaEnv(lua) {}
    public:
        void Invoke(const com::Rpc_Response & backData) override;

    private:
		int ref;
		lua_State *luaEnv;
		lua_State * mCoroutine;
    };

    class CoroutineComponent;

    class CppCallHandler : public CallHandler
    {
    public:
        CppCallHandler(int method);
		~CppCallHandler() final;
    public:
        void Invoke(const com::Rpc_Response & backData) override;
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