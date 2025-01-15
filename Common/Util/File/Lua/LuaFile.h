//
// Created by leyi on 2023/6/29.
//

#ifndef APP_LUAFILE_H
#define APP_LUAFILE_H
#include"Lua/Lib/Lib.h"
namespace lua
{
	namespace LuaFile
	{
		int Find(lua_State * lua);
		int GetMd5(lua_State * lua);
		int IsExist(lua_State * lua);
		int GetFiles(lua_State * lua);
		int GetFileName(lua_State * lua);
		int GetLastWriteTime(lua_State * lua);
	};

	namespace LuaDir
	{
		int Make(lua_State * lua);
		int IsExist(lua_State * lua);
	}
}


#endif //APP_LUAFILE_H
