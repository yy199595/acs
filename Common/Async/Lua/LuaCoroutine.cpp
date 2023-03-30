#include"LuaCoroutine.h"
#include"App/App.h"
#include"Log/Debug.h"
#include"Timer/TimerBase.h"
#include"Lua/LuaWaitTaskSource.h"
#include"Component/TimerComponent.h"
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
		int ms = (int)luaL_checkinteger(lua, 1);
		TimerComponent * timerComponent = App::Inst()->GetTimerComponent();
		std::shared_ptr<LuaWaitTaskSource> luaRpcTaskSource(new LuaWaitTaskSource(lua));
		timerComponent->DelayCall(ms, [luaRpcTaskSource]()
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
        Coroutine::Resume(coroutine, lua, size -1);
		return 1;
	}

    void Coroutine::Resume(lua_State *cor, lua_State *lua, int args)
    {
        int code = lua_resume(cor, lua, args);
        if(code != LUA_OK && code != LUA_YIELD)
        {
			Debug::LuaError(lua_tostring(cor, -1));
        }
    }
}
