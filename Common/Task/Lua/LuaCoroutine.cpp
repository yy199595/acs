#include"LuaCoroutine.h"
#include"App/App.h"
#include"Lua/luadebug.h"
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
        Coroutine::Resume(coroutine, lua, size -1);
		return 1;
	}

    void Coroutine::Resume(lua_State *cor, lua_State *lua, int args)
    {
        int code = lua_resume(cor, lua, args);
        if(code != LUA_OK && code != LUA_YIELD)
        {
            LuaDebug::onError(cor);
//            const char * err = lua_tostring(cor, -1);
//            lua_pushcfunction(lua, Lua::Log::DebugError);
//            lua_pushstring(lua, err);
//            lua_pcall(lua, 1, 0, 0);
        }
    }
}
