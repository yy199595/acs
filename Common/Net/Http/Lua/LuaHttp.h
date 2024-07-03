//
// Created by mac on 2022/5/30.
//

#ifndef APP_LUAHTTP_H
#define APP_LUAHTTP_H
#include"Lua/Engine/Define.h"
namespace Lua
{
	namespace HttpClient
	{
		extern int Do(lua_State* lua);
		extern int Get(lua_State* lua);
		extern int Post(lua_State* lua);
		extern int Upload(lua_State* lua);
		extern int Download(lua_State* lua);
	}

	namespace HttpHead
	{
		extern int Get(lua_State * L);
	}
}


#endif //APP_LUAHTTP_H
