#include "LuaManager.h"

namespace Lua
{
	void LuaManager::AddRequirePath(const std::string path)
	{
		lua_getglobal(luaEnv, "package");
		if (lua_istable(luaEnv, -1))
		{
			lua_getfield(luaEnv, -1, "path");
			if (lua_isstring(luaEnv, -1))
			{
				const char* packagePath = lua_tostring(luaEnv, -1);
				char buffer[2048] = { 0 };
#ifdef _MSC_VER
				size_t size = sprintf_s(buffer, "%s;%s", packagePath, path.c_str());
#else
				size_t size = sprintf(buffer, "%s;%s", packagePath, path.c_str());
#endif
				lua_pushlstring(luaEnv, buffer, size);
				lua_setfield(luaEnv, -2, "path");
			}
		}
	}

	bool LuaManager::LoadLuaFile(const std::string filePath)
	{
		lua_pushcfunction(luaEnv, LuaDebugStack::PrintErrorInfo);
		int ref = lua_gettop(luaEnv);
		if (luaL_loadfile(luaEnv, filePath.c_str()) == 0)
		{
			int code = lua_pcallk(luaEnv, 0, 0, ref, 0, 0);
			return code == 0;
		}
		return false;
	}
}
