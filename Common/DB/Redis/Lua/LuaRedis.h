//
// Created by yjz on 2022/6/15.
//

#ifndef APP_LUAREDIS_H
#define APP_LUAREDIS_H
#include"Lua/Engine/Define.h"
namespace lua
{
	namespace redis
	{
		extern int Run(lua_State * lua);
		extern int Call(lua_State * lua);
		extern int Send(lua_State * lua);
	};

	namespace sub_redis
	{
		extern int Run(lua_State* lua);
	}
}

#endif //APP_LUAREDIS_H
