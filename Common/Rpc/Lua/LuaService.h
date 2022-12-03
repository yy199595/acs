//
// Created by yjz on 2022/4/5.
//

#ifndef _SERVICE_H_
#define _SERVICE_H_
#include"Lua/LuaInclude.h"
#include"Service/RpcService.h"
namespace Lua
{
	namespace Service
	{
		int Call(lua_State * lua);
		int Send(lua_State* lua);
        int AddLocation(lua_State * lua);
        int AllotServer(lua_State * lua);
        int GetServerList(lua_State* lua);
    }
}

#endif //_SERVICE_H_
