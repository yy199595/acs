#pragma once
#include "ServiceMethod.h"
struct lua_State;
namespace Sentry
{

	class ProtoConfig;
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
		tuple<XCode, std::shared_ptr<Message>> Call(long long id, const std::string& json);
		tuple<XCode, std::shared_ptr<Message>> CallAsync(long long id, const std::string& json);
	 private:
		lua_State* mLuaEnv;
		std::string mFunction;
	};
}