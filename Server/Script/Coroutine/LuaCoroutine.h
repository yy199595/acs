//
// Created by yjz on 2022/4/5.
//

#ifndef _LUACOROUTINE_H_
#define _LUACOROUTINE_H_
#include"Script/LuaInclude.h"
#include "Timer/TimerBase.h"

namespace Sentry
{
	class LuaSleepTimer : public TimerBase
	{
	 public:
		LuaSleepTimer(lua_State *lua, int ref, long long ms);

		~LuaSleepTimer() final { luaL_unref(mLuaEnv, LUA_REGISTRYINDEX, this->mRef); }

		static std::shared_ptr<LuaSleepTimer> Create(lua_State *lua, int index, long long ms);

	 public:
		void Invoke(TimerState state = TimerState::Ok) override;
	 private:
		int mRef;
		lua_State *mLuaEnv;
	};
}

namespace Lua
{
	namespace Coroutine
	{
		extern int Sleep(lua_State * lua);
		extern int Start(lua_State * lua);
	}
}

#endif //_LUACOROUTINE_H_
