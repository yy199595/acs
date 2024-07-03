//
// Created by leyi on 2023/6/29.
//

#ifndef APP_LUAFILE_H
#define APP_LUAFILE_H
#include"Lua/Engine/Define.h"
namespace Lua
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
}


#endif //APP_LUAFILE_H
