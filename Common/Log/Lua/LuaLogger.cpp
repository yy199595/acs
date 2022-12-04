//
// Created by yjz on 2022/4/4.
//

#include"LuaLogger.h"
#include"App/App.h"
#include"String/StringHelper.h"
#include"Component/LoggerComponent.h"
using namespace Sentry;
namespace Lua
{
	namespace Log
	{
		extern void GetLuaString(lua_State* luaEnv, std::string & ret)
		{
			lua_Debug ar;
			if (lua_getstack(luaEnv, 1, &ar) == 1)
			{
				lua_getinfo(luaEnv, "nSlu", &ar);
				int n = lua_gettop(luaEnv);
				lua_getglobal(luaEnv, "tostring");
				std::stringstream stringBuffer;

				std::string fillName;
				Helper::String::GetFileName(ar.short_src, fillName);
				stringBuffer << fillName << ":" << ar.currentline << "  ";
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
				ret = stringBuffer.str();
			}
		}

		int DebugLog(lua_State* luaEnv)
		{
			std::string log;
			GetLuaString(luaEnv, log);
			App::Inst()->GetLogger()->AddLog(spdlog::level::debug, log);
			return 0;
		}

		int DebugInfo(lua_State* luaEnv)
		{
			std::string log;
			GetLuaString(luaEnv, log);
			App::Inst()->GetLogger()->AddLog(spdlog::level::info, log);
			return 0;
		}

		int SystemError(lua_State* luaEnv)
		{
			size_t size = 0;
			const char* str = luaL_checklstring(luaEnv, 1, &size);
			if(str != nullptr && size > 0)
			{
				std::string log(str, size);
				size_t pos = log.find_last_of('/');
				if(pos != std::string::npos)
				{
					log = log.substr(pos + 1);
					lua_pushlstring(luaEnv, log.c_str(), log.size());
					App::Inst()->GetLogger()->AddLog(spdlog::level::err, log);
					return 1;
				}
			}
			return 0;
		}

		int DebugError(lua_State* luaEnv)
		{
			std::string log;
			GetLuaString(luaEnv, log);
			App::Inst()->GetLogger()->AddLog(spdlog::level::err, log);
			return 0;
		}

		int DebugWarning(lua_State* luaEnv)
		{
			std::string log;
			GetLuaString(luaEnv, log);
			App::Inst()->GetLogger()->AddLog(spdlog::level::warn, log);
			return 0;
		}
	}

}