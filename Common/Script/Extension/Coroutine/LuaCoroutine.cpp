#include"LuaCoroutine.h"
#include"App/App.h"
#include"Define/CommonLogDef.h"
#include"Async/RpcTask/RpcTaskSource.h"
#include"Component/Timer/TimerComponent.h"
using namespace Sentry;
namespace Lua
{
	int Coroutine::Sleep(lua_State* lua)
	{
		if(!lua_isnumber(lua, 1))
		{
			luaL_error(lua, "first parameter must number");
			return 0;
		}
		lua_pushthread(lua);
		float second = lua_tonumber(lua, 1);
		TimerComponent * timerComponent = App::Get()->GetTimerComponent();
		std::shared_ptr<LuaRpcTaskSource> luaRpcTaskSource(new LuaRpcTaskSource(lua));
		long long id = timerComponent->DelayCall(second, [luaRpcTaskSource]()
		{
			luaRpcTaskSource->SetResult();
		});
		lua_pushinteger(lua, id);
		return luaRpcTaskSource->Yield();
	}

	int Coroutine::Start(lua_State* lua)
	{
		if(!lua_isfunction(lua, 1))
		{
			luaL_error(lua, "parameter must function");
			return 0;
		}
		lua_State* coroutine = lua_newthread(lua);
		lua_pushvalue(lua, 1);
		lua_xmove(lua, coroutine, 1);
		lua_replace(lua, 1);

		const int size = lua_gettop(lua);
		lua_xmove(lua, coroutine, size - 1);
		lua_presume(coroutine, lua, size - 1);
		return 1;
	}
}
