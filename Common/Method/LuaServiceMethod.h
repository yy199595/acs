#pragma once
#include"ServiceMethod.h"
#include<google/protobuf/message.h>
using namespace google::protobuf;
struct lua_State;
namespace Sentry
{

	class RpcInterfaceConfig;
	class LuaServiceMethod : public ServiceMethod
	{
	 public:
		LuaServiceMethod(const RpcInterfaceConfig * config, lua_State* lua);
	 public:
		bool IsLuaMethod() final { return true; }
		XCode Invoke(const com::Rpc_Request& request, com::Rpc_Response& response) final;
	 private:
		XCode Call(int count, com::Rpc::Response & response);
		XCode CallAsync(int count, com::Rpc::Response & response);
	 private:
		lua_State* mLuaEnv;
        const RpcInterfaceConfig * mConfig;
		class MessageComponent * mMsgComponent;
	};
}