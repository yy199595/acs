//
// Created by yjz on 2022/4/4.
//

#include"LuaLogger.h"
#include"Entity/Actor/App.h"
#include"Util/Tools/String.h"

using namespace acs;
namespace Lua
{

	void Log::Error(lua_State * lua)
	{
		size_t size = 0;
		const char * str = lua_tolstring(lua, -1, &size);
		std::unique_ptr<custom::LogInfo> logInfo = std::make_unique<custom::LogInfo>();
		{
			if(str != nullptr && size > 0)
			{
				std::string error(str, size);
				size_t pos = error.find(' ');
				if(pos != std::string::npos)
				{
					std::string path = error.substr(0, pos - 1);
					size_t pos1 = path.find_last_of('/');
					if(pos1 != std::string::npos)
					{
						path = path.substr(pos1 + 1);
					}
					logInfo->File = path;
					logInfo->Content = error.substr(pos + 1);
				}
			}
			else
			{
				logInfo->Content.append(str, size);
			}
			logInfo->Level = custom::LogLevel::Error;
		}
		Debug::Log(std::move(logInfo));
	}

	int Log::Output(lua_State* lua)
	{
		lua_Debug luaDebug;
		std::unique_ptr<custom::LogInfo> logInfo = std::make_unique<custom::LogInfo>();
		if(lua_getstack(lua, 2, &luaDebug) > 0)
		{
			lua_getinfo(lua, "nSlu", &luaDebug); // 获取当前函数名和行号等信息
			logInfo->File = FormatFileLine(luaDebug.short_src, luaDebug.currentline);
		}
		size_t size = 0;
		int type = (int)luaL_checkinteger(lua, 1);
		{
			logInfo->Level = (custom::LogLevel)type;
			const char* log = luaL_checklstring(lua, 2, &size);
			logInfo->Content.append(log, size);
		}
		Debug::Log(std::move(logInfo));
		return 0;
	}

	int Log::OnCallError(lua_State* lua)
	{
		assert(false);
		return 0;
	}

	int Console::Show(lua_State* lua)
	{
		lua_Debug luaDebug;
		custom::LogInfo logInfo;
		if(lua_getstack(lua, 2, &luaDebug) > 0)
		{
			lua_getinfo(lua, "nSlu", &luaDebug); // 获取当前函数名和行号等信息
			logInfo.File = FormatFileLine(luaDebug.short_src, luaDebug.currentline);
		}
		size_t size = 0;
		int type = (int)luaL_checkinteger(lua, 1);
		{
			logInfo.Level = (custom::LogLevel)type;
			const char* log = luaL_checklstring(lua, 2, &size);
			logInfo.Content.append(log, size);
		}
		Debug::Console(logInfo);
		return 0;
	}
}
