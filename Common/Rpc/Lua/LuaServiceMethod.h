#pragma once
#include"Client/Message.h"
#include"Method/ServiceMethod.h"
#include<google/protobuf/message.h>
using namespace google::protobuf;
struct lua_State;
namespace Sentry
{

	class RpcMethodConfig;
	class LuaServiceMethod : public ServiceMethod
	{
	 public:
		LuaServiceMethod(const RpcMethodConfig * config);
	 public:
		bool IsLuaMethod() final { return true; }
		XCode Invoke(Rpc::Packet & message) final;
	 private:
		XCode Call(int count, Rpc::Packet & message);
		XCode CallAsync(int count, Rpc::Packet & message);
	 private:
		lua_State* mLuaEnv;
        const RpcMethodConfig * mConfig;
		class ProtoComponent * mMsgComponent;
		class LuaScriptComponent * mLuaComponent;
	};
}