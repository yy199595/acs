//
// Created by yjz on 2022/4/5.
//

#ifndef _SERVICE_H_
#define _SERVICE_H_
#include"Lua/LuaInclude.h"
#include"Service/Service.h"
namespace Lua
{
	namespace Service
	{
		int Call(lua_State * lua);
		int GetLocation(lua_State * lua);
        int AddLocation(lua_State * lua);
	}
}

#endif //_SERVICE_H_
