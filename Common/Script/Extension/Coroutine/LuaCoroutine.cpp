#include"LuaCoroutine.h"
#include"App/App.h"
#include"Define/CommonLogDef.h"
#include"Async/Lua/LuaWaitTaskSource.h"
#include"Component/Timer/TimerComponent.h"
using namespace Sentry;
#define Await(task) task->Await()
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
		float second = luaL_checknumber(lua, 1);
		TimerComponent * timerComponent = App::Get()->GetTimerComponent();
		std::shared_ptr<LuaWaitTaskSource> luaRpcTaskSource(new LuaWaitTaskSource(lua));
		timerComponent->DelayCall(second, [luaRpcTaskSource]()
		{
			luaRpcTaskSource->SetResult();
		});
		return Await(luaRpcTaskSource);
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
