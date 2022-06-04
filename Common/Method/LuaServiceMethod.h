#pragma once
#include"ServiceMethod.h"
#include<google/protobuf/message.h>
using namespace google::protobuf;
struct lua_State;
namespace Sentry
{

	class InterfaceConfig;
	class LuaServiceMethod : public ServiceMethod
	{
	 public:
		LuaServiceMethod(const std::string & service, const std::string & func, lua_State* lua);
	 public:
		bool IsLuaMethod() final
		{
			return true;
		}
		XCode Invoke(const com::Rpc_Request& request, com::Rpc_Response& response) final;
	 private:
		XCode Call(long long id, std::shared_ptr<Message> message, com::Rpc::Response & response);
		XCode CallAsync(long long id, std::shared_ptr<Message> message, com::Rpc::Response & response);
	 private:
		lua_State* mLuaEnv;
		std::string mFunction;
		class MessageComponent * mMsgComponent;
		class ServiceComponent * mServiceCompoent;
	};
}