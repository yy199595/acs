//
// Created by mac on 2022/5/16.
//

#ifndef SERVER_JSON_H
#define SERVER_JSON_H
#include"Script/LuaInclude.h"
namespace Lua
{
	namespace Json
	{
		extern int Encode(lua_State * lua);
		extern int Decode(lua_State * lua);
	};
}

#endif //SERVER_JSON_H
