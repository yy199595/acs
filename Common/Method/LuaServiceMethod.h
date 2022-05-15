#pragma once
#include "ServiceMethod.h"
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
		XCode Call(long long id, const std::string& json, com::Rpc::Response & response);
		XCode CallAsync(long long id, const std::string& json, com::Rpc::Response & response);
	 private:
		lua_State* mLuaEnv;
		std::string mFunction;
	};
}