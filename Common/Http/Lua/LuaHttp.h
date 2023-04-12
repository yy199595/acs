//
// Created by mac on 2022/5/30.
//

#ifndef SERVER_LUAHTTP_H
#define SERVER_LUAHTTP_H
#include"Lua/Engine/Define.h"
namespace Lua
{
	namespace HttpClient
	{
		extern int Get(lua_State* lua);
		extern int Post(lua_State* lua);
		extern int Download(lua_State* lua);
	}
}


#endif //SERVER_LUAHTTP_H
