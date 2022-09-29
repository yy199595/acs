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
		LuaServiceMethod(const RpcMethodConfig * config, lua_State* lua);
	 public:
		bool IsLuaMethod() final { return true; }
		XCode Invoke(Rpc::Data & message) final;
	 private:
		XCode Call(int count, Rpc::Data & message);
		XCode CallAsync(int count, Rpc::Data & message);
	 private:
		lua_State* mLuaEnv;
        const RpcMethodConfig * mConfig;
		class ProtoComponent * mMsgComponent;
	};
}