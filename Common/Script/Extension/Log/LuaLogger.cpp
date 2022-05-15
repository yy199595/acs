//
// Created by yjz on 2022/4/4.
//

#include"LuaLogger.h"
#include"App/App.h"
#include"Util/StringHelper.h"
#include"Component/Scene/LoggerComponent.h"
using namespace Sentry;
namespace Lua
{
	namespace Log
	{
		extern std::string GetLuaString(lua_State* luaEnv)
		{
			lua_Debug ar;
			if (lua_getstack(luaEnv, 1, &ar) == 1)
			{
				lua_getinfo(luaEnv, "nSlu", &ar);
				int n = lua_gettop(luaEnv);
				lua_getglobal(luaEnv, "tostring");
				std::stringstream stringBuffer;

				stringBuffer << Helper::String::GetFileName(ar.short_src)
							 << ":" << ar.currentline << "  ";
				for (int i = 1; i <= n; i++)
				{
					size_t size;
					lua_pushvalue(luaEnv, -1);
					lua_pushvalue(luaEnv, i);
					lua_call(luaEnv, 1, 1);
					const char* str = lua_tolstring(luaEnv, -1, &size);
					lua_pop(luaEnv, 1);
					stringBuffer << str << " ";
				}
				return stringBuffer.str();
			}
			return std::string();
		}

		int DebugLog(lua_State* luaEnv)
		{
			const std::string log = GetLuaString(luaEnv);
			App::Get()->GetLogger()->AddLog(spdlog::level::debug, log);
			return 0;
		}

		int DebugInfo(lua_State* luaEnv)
		{
			const std::string log = GetLuaString(luaEnv);
			App::Get()->GetLogger()->AddLog(spdlog::level::info, log);
			return 0;
		}

		int DebugError(lua_State* luaEnv)
		{
			const std::string log = GetLuaString(luaEnv);
			App::Get()->GetLogger()->AddLog(spdlog::level::err, log);
			return 0;
		}

		int DebugWarning(lua_State* luaEnv)
		{
			LoggerComponent* loggerComponent = App::Get()->GetLogger();
			loggerComponent->AddLog(spdlog::level::warn, GetLuaString(luaEnv));
			return 0;
		}
	}

}