//
// Created by mac on 2022/5/30.
//

#pragma once
#include"Lua/Engine/Define.h"
namespace lua
{
	namespace Status
	{
		constexpr int Ok = 0;
		constexpr int Timeout = 1;
		constexpr int ReadError = 2;
		constexpr int SendError = 3;
		constexpr int AcceptError = 4;
	}

	namespace TcpSock
	{
		extern int Send(lua_State*);
		extern int Read(lua_State*);
		extern int Query(lua_State*);
		extern int Close(lua_State*);
		extern int Connect(lua_State*);
		extern int Listen(lua_State *);
		extern int Accept(lua_State*);
		extern int SetTimeout(lua_State*);
	}
}
