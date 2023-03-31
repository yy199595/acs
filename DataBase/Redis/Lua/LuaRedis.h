//
// Created by yjz on 2022/6/15.
//

#ifndef _LUAREDIS_H_
#define _LUAREDIS_H_
#include"Script/Lua/LuaInclude.h"
namespace Sentry
{
	class RedisRequest;
	class TcpRedisClient;
	class RedisComponent;
}

using namespace Sentry;
namespace Lua
{
	namespace Redis
	{
		int Run(lua_State * lua);
		int Call(lua_State * lua);
        int Send(lua_State * lua);
	};
}

#endif //_LUAREDIS_H_
