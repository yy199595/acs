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

		int Debug(lua_State* luaEnv)
		{
			std::string log;
			GetLuaString(luaEnv, log);
			Debug::Log(Debug::Level::debug, log);
			App::Inst()->GetLogger()->PushLog(spdlog::level::debug, log);
			return 0;
		}

		int Info(lua_State* luaEnv)
		{
			std::string log;
			GetLuaString(luaEnv, log);
			Debug::Log(Debug::Level::info, log);
			App::Inst()->GetLogger()->PushLog(spdlog::level::info, log);
			return 0;
		}

		int LuaError(lua_State* luaEnv)
		{
			const char* log = luaL_checkstring(luaEnv, 1);
            LUA_LOG_ERROR(log);
			return 0;
		}

        int Error(lua_State* luaEnv)
		{
			std::string log;
			GetLuaString(luaEnv, log);
			Debug::Log(Debug::Level::err, log);
			App::Inst()->GetLogger()->PushLog(spdlog::level::err, log);
			return 0;
		}

		int Warning(lua_State* luaEnv)
		{
			std::string log;
			GetLuaString(luaEnv, log);
			Debug::Log(Debug::Level::warn, log);
			App::Inst()->GetLogger()->PushLog(spdlog::level::warn, log);
			return 0;
		}
	}

	int Console::Debug(lua_State* luaEnv)
	{
		std::string log;
		Log::GetLuaString(luaEnv, log);
		Debug::Log(Debug::Level::debug, log);
		return 0;
	}

	int Console::Info(lua_State* luaEnv)
	{
		std::string log;
		Log::GetLuaString(luaEnv, log);
		Debug::Log(Debug::Level::info, log);
		return 0;
	}

	int Console::Error(lua_State* luaEnv)
	{
		std::string log;
		Log::GetLuaString(luaEnv, log);
		Debug::Log(Debug::Level::err, log);
		return 0;
	}

	int Console::Warning(lua_State* luaEnv)
	{
		std::string log;
		Log::GetLuaString(luaEnv, log);
		Debug::Log(Debug::Level::warn, log);
		return 0;
	}

}
