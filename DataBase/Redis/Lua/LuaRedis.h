//
// Created by yjz on 2022/6/15.
//

#ifndef _LUAREDIS_H_
#define _LUAREDIS_H_
#include"Script/Lua/LuaInclude.h"
namespace Tendo
{
	class RedisRequest;
	class TcpRedisClient;
	class RedisComponent;
}

using namespace Tendo;
namespace Lua
{
	namespace Redis
	{
		int Run(lua_State * lua);
		int Call(lua_State * lua);
        int Send(lua_State * lua);
		int SyncRun(lua_State* lua);
	};
}

#endif //_LUAREDIS_H_
