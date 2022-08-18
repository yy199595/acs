//
// Created by yjz on 2022/4/5.
//

#ifndef _SERVICE_H_
#define _SERVICE_H_
#include"Script/LuaInclude.h"
#include"Component/RpcService/Service.h"
namespace Lua
{
	namespace Service
	{
		int Call(lua_State * lua);
		int GetHost(lua_State * lua);
        int AddHost(lua_State * lua);
	}
}

#endif //_SERVICE_H_
