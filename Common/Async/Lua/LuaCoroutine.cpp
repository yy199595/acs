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
		if (lua == nullptr)
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
		LuaWaitTaskSource* luaRpcTaskSource = new LuaWaitTaskSource(lua);
		timerComponent->DelayCall(ms, [luaRpcTaskSource]()
		{
			luaRpcTaskSource->SetResult();
			delete luaRpcTaskSource;
		});
		return luaRpcTaskSource->Await();
	}

	int Coroutine::Start(lua_State* lua)
	{
		// 确保传入的参数是一个函数
		if (!lua_isfunction(lua, 1))
		{
			luaL_error(lua, "parameter must be a function");
			return 0;
		}

		// 创建新的协程
		lua_State* coroutine = lua_newthread(lua);

		lua_pushvalue(lua, 1);
		lua_xmove(lua, coroutine, 1);  // 将函数从主栈移动到协程栈

		const int size = lua_gettop(lua);  // 获取当前栈的大小
		lua_xmove(lua, coroutine, size - 1);  // 将除去函数外的其他参数移动到协程栈
		Coroutine::Resume(coroutine, lua, size - 1);
		return 1;
	}

	void Coroutine::Resume(lua_State* cor, lua_State* lua, int args)
	{
		int ret = 0;
		int code = lua_status(cor);
		switch (lua_status(cor))
		{
			case LUA_OK:
			LOG_ERROR("Coroutine already finished.");
				break;
			case LUA_YIELD:
			{
				code = lua_resume(cor, lua, args, &ret);
				if (code != LUA_OK && code != LUA_YIELD)
				{
					size_t size = 0;
					const char* err = lua_tolstring(lua, -1, &size);
					std::string errorMsg(err, size);

					// 记录错误信息
					std::unique_ptr<custom::LogInfo> logInfo = std::make_unique<custom::LogInfo>();
					logInfo->Level = custom::LogLevel::Error;
					logInfo->Content = "Error during coroutine resume: " + errorMsg;
					Debug::Log(std::move(logInfo));
					lua_pop(lua, 1);
				}
				if (ret > 0)
				{
					lua_pop(lua, ret);
				}
				break;
			}
			default:
			LOG_ERROR("coroutine status:{}", code);
				break;
		}
	}
}
