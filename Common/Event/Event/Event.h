//
// Created by leyi on 2023/6/6.
//

#ifndef APP_EVENT_H
#define APP_EVENT_H
#include<string>
#include<functional>
#include"Lua/Engine/LuaParameter.h"
namespace Tendo
{
	class IEvent
	{
	public:
		virtual bool IsLuaEvent() = 0;
	};

	template<typename ...Args>
	class CEvent : public IEvent
	{
		typedef std::function<void(const Args...)> Callback;
	public:
		CEvent(Callback && callback) : mCallback(callback) { }
	public:
		bool IsLuaEvent() { return false; }
		void Invoke(const Args&&... args) { this->mCallback(std::forward<Args>(args)...); }
	private:
		Callback mCallback;
	};

	template<typename ... Args>
	class LuaEvent : public IEvent
	{
	public:
		LuaEvent(lua_State * lua) : ref(0), mLua(lua) { }
	public:
		bool Init();
		bool IsLuaEvent() { return true; }
		void Invoke(const Args && ... args);
	private:
		int ref;
		lua_State * mLua;
	};
	template<typename ... Args>
	bool LuaEvent<Args...>::Init()
	{
		this->ref = 0;
		if(lua_isfunction(this->mLua, -1))
		{
			this->ref = luaL_ref(this->mLua, LUA_REGISTRYINDEX);
			return true;
		}
		return false;
	}
	template<typename ... Args>
	void LuaEvent<Args...>::Invoke(const Args&& ...args)
	{
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mLua);
		Lua::Parameter::WriteArgs(this->mLua, std::forward<Args>(args)...);
		if (lua_pcall(this->mLua, sizeof...(Args), 0, 0) != 0)
		{
			printf("[lua error ] : %s", lua_tostring(this->mLua, -1));
		}
	}
}
#endif //APP_EVENT_H
