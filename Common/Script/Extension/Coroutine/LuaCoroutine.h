//
// Created by yjz on 2022/4/5.
//

#ifndef _LUACOROUTINE_H_
#define _LUACOROUTINE_H_
#include"Script/LuaInclude.h"
#include "Timer/TimerBase.h"

namespace Lua
{
	namespace Coroutine
	{
		extern int Sleep(lua_State * lua);
		extern int Start(lua_State * lua);
	}
}

#endif //_LUACOROUTINE_H_
