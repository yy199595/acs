//
// Created by mac on 2022/5/30.
//

#ifndef APP_LUASOCKET_H
#define APP_LUASOCKET_H
#include"Lua/Engine/Define.h"
namespace Lua
{
	namespace TcpSock
	{
		extern int Send(lua_State*);
		extern int Read(lua_State*);
		extern int Query(lua_State*);
		extern int Close(lua_State*);
		extern int Connect(lua_State*);
	}

	namespace UdpSock
	{
		extern int New(lua_State*);
		extern int Send(lua_State*);
	}

	namespace KcpSock
	{
		extern int New(lua_State *);
		extern int Send(lua_State *);
	}
}


#endif
