//
// Created by leyi on 2023/6/14.
//

#ifndef APP_LUACOROUTINE_H
#define APP_LUACOROUTINE_H
#include"Lua/Engine/LuaParameter.h"
#include "Log/Common/CommonLogDef.h"
#include"Async/Lua/WaitLuaTaskSource.h"
namespace Lua
{
	class LuaCoroutine
	{
	public:
		LuaCoroutine(lua_State * lua);
		~LuaCoroutine() { luaL_unref(this->mLua, LUA_REGISTRYINDEX, ref);}
	public:
		template<typename... Args>
		tendo::WaitLuaTaskSource * Await(const Args&& ... args);
	private:
		inline bool GetFunction(const char * name);
	private:
		int ref;
		lua_State * mLua;
	};

	LuaCoroutine::LuaCoroutine(lua_State* lua)
	{
		lua_getglobal(lua, "coroutine");
		this->ref = luaL_ref(mLua, LUA_REGISTRYINDEX);
	}

	template<typename... Args>
	tendo::WaitLuaTaskSource * LuaCoroutine::Await(const Args&& ...args)
	{
		if(!lua_isfunction(this->mLua, -1))
		{
			return nullptr;
		}
		if(!this->GetFunction("call"))
		{
			return nullptr;
		}
		lua_pushvalue(this->mLua, -3);
		lua_pushvalue(this->mLua, -2);
		Parameter::WriteArgs<Args...>(this->mLua, std::forward<Args>(args)...);
		if (lua_pcall(this->mLua, sizeof...(Args) + 2, 1, 0) != LUA_OK)
		{
			LOG_ERROR(lua_tostring(this->mLua, -1));
			return nullptr;
		}
		return Lua::PtrProxy<tendo::WaitLuaTaskSource>::Read(this->mLua, -1);
	}

	bool LuaCoroutine::GetFunction(const char* name)
	{
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->ref);
		lua_getfield(this->mLua, -1, name);
		return lua_isfunction(this->mLua, -1);
	}
}

#endif //APP_LUACOROUTINE_H
