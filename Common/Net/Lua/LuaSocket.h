//
// Created by mac on 2022/5/30.
//

#ifndef APP_LUASOCKET_H
#define APP_LUASOCKET_H
#include"Lua/Engine/Define.h"
namespace Lua
{
	namespace Sock
	{
		extern int Send(lua_State*);
		extern int Read(lua_State*);
		extern int Query(lua_State*);
		extern int Connect(lua_State*);
	}
}


#endif //APP_LUAHTTP_H
