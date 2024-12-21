//
// Created by yjz on 2022/4/5.
//

#ifndef APP_LUACOROUTINE_H
#define APP_LUACOROUTINE_H
#include"Lua/Engine/Define.h"
namespace Lua
{
    namespace Coroutine
	{
		extern int Sleep(lua_State * lua);
		extern bool IsRunning(lua_State * lua);
		extern void Resume(lua_State * cor, int args);
	}
}

#endif //APP_LUACOROUTINE_H
