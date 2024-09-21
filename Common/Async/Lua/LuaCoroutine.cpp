#include"LuaCoroutine.h"
#include"Entity/Actor/App.h"
#include"Log/Lua/LuaLogger.h"
#include"Lua/Engine/LuaDebugStack.h"
#include"Async/Lua/LuaWaitTaskSource.h"
#include"Timer/Component/TimerComponent.h"

using namespace acs;

namespace Lua
{
	bool Coroutine::IsRunning(lua_State* lua)
	{
        if(lua == nullptr)
		{
			return false;
		}
		int top = lua_gettop(lua);
		return top == 1 && lua_isthread(lua, 1);
	}
	int Coroutine::Sleep(lua_State* lua)
	{
		static TimerComponent* timerComponent = nullptr;
		if (timerComponent == nullptr)
		{
			timerComponent = App::Get<TimerComponent>();
			if (timerComponent == nullptr)
			{
				luaL_error(lua, "not find TimerComponent");
				return 0;
			}
		}

		if (!lua_isnumber(lua, 1))
		{
			luaL_error(lua, "first parameter must number");
			return 0;
		}
		lua_pushthread(lua);
		int ms = (int)luaL_checkinteger(lua, 1);
		LuaWaitTaskSource * luaRpcTaskSource = new LuaWaitTaskSource(lua);
		timerComponent->DelayCall(ms, [luaRpcTaskSource]()
		{
			luaRpcTaskSource->SetResult();
			delete luaRpcTaskSource;
		});
		return luaRpcTaskSource->Await();
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
		int ret = 0;
        int code = lua_resume(cor, lua, args, &ret);
        if(code != LUA_OK && code != LUA_YIELD)
        {
			size_t size = 0;
			const char * str = lua_tolstring(lua, -1, &size);
			std::unique_ptr<custom::LogInfo> logInfo = std::make_unique<custom::LogInfo>();
			{
				std::string error(str, size);
				size_t pos = error.find(".lua");
				logInfo->Level = custom::LogLevel::Error;
				if(pos != std::string::npos)
				{
					logInfo->Content.append(str, size);
				}
				else
				{
					std::string path = Lua::LuaDebugStack::GetCurrentStack(cor);
					logInfo->Content.append(path);
					logInfo->Content.append(str, size);
				}

				Debug::Log(std::move(logInfo));
			}
        }
    }
}
