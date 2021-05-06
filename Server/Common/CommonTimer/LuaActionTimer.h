#pragma once
#include"TimerBase.h"
#include<CommonScript/LuaInclude.h>
namespace SoEasy
{
	class LuaActionTimer : public TimerBase
	{
	public:
		LuaActionTimer(lua_State * luaEnv, int ref, int interval, int count = 1);
		~LuaActionTimer();
	public:
		bool Invoke() override; //触发之后执行的操作
	private:
		int mRef;
		int mInterval;
		int mInvokeCount;
		int mMaxInvokeCount;
		lua_State * mLuaEnv;
	};
}