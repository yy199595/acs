#include"LuaRpcService.h"
#include"Lua/Module/LuaModule.h"
namespace joke
{
	void LuaRpcService::OnLogin(long long playerId)
	{
		static const std::string func("_OnLogin");
		Lua::LuaModule * luaModule = this->GetLuaModule();
		if(luaModule->HasFunction(func))
		{
			luaModule->Await(func, playerId);
		}
	}

	void LuaRpcService::OnLogout(long long playerId)
	{
		static const std::string func("_OnLogout");
		Lua::LuaModule * luaModule = this->GetLuaModule();
		if(luaModule->HasFunction(func))
		{
			luaModule->Await(func, playerId);
		}
	}
}