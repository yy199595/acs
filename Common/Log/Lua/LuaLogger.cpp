//
// Created by yjz on 2022/4/4.
//

#include"LuaLogger.h"
#include"Entity/Unit/App.h"
#include"Util/String/StringHelper.h"
#include"Log/Component/LogComponent.h"

using namespace Tendo;
namespace Lua
{

	int Log::Output(lua_State* lua)
	{
		size_t size = 0;
		int type = (int)luaL_checkinteger(lua, 1);
		const char* log = luaL_checklstring(lua, 2, &size);
		LogComponent* logComponent = App::Inst()->GetLogger();
		{
			std::string message(log, size);
			logComponent->SaveLog((Debug::Level)type, message);
		}
		return 0;
	}

	int Console::Show(lua_State* lua)
	{
		size_t size = 0;
		int type = (int)luaL_checkinteger(lua, 1);
		const char* log = luaL_checklstring(lua, 2, &size);
		Debug::Log((Debug::Level)type, std::string(log, size));
		return 0;
	}
}
