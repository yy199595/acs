//
// Created by 64658 on 2024/11/21.
//

#include "LuaOs.h"
#include "Core/System/System.h"
namespace LuaCore
{
	void lua_set_number_field(lua_State * L, const char * k, double number)
	{
		lua_pushnumber(L, number);
		lua_setfield(L, -2, k);
	}

	int GetSystemInfo(lua_State* L)
	{
		os::SystemInfo systemInfo;
		os::System::GetSystemInfo(systemInfo);
		{
			lua_newtable(L);
			lua_set_number_field(L, "cpu", systemInfo.cpu);
			lua_set_number_field(L, "use_memory", systemInfo.use_memory);
			lua_set_number_field(L, "max_memory", systemInfo.max_memory);
		}
		return 1;
	}
}