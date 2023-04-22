//
// Created by yjz on 2022/4/5.
//

#ifndef _SERVICE_H_
#define _SERVICE_H_
#include"Lua/Engine/Define.h"
namespace Lua
{
	namespace Service
	{
		int Call(lua_State * lua);
		int Send(lua_State* lua);
		int RangeServer(lua_State * lua); //随机一个服务器
		int GetAddrById(lua_State * lua); //根据服务器id获取地址
        int FindService(lua_State* lua);
        int AddLocation(lua_State * lua);
        int AllotServer(lua_State * lua);
        int GetServerList(lua_State* lua);
    }
}

#endif //_SERVICE_H_
