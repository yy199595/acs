//
// Created by mac on 2022/5/31.
//

#ifndef SERVER_CLIENT_H
#define SERVER_CLIENT_H
#include"Script/Lua/LuaInclude.h"

namespace Lua
{
	namespace ClientEx
	{
		int New(lua_State* lua);
		int Call(lua_State * lua);
	};
}


#endif //SERVER_CLIENT_H
