#pragma once

#include"App/App.h"
#include"Lua/LuaParameter.h"
#include"Define/CommonLogDef.h"
#include"Lua/WaitLuaTaskSource.h"
using namespace Sentry;

namespace Lua
{
	class Function
	{
	 public:
		Function(lua_State* luaEvn, int ref);

		~Function()
		{
			luaL_unref(luaEnv, LUA_REGISTRYINDEX, this->ref);
		}

	 public:
		int GetRef()
		{
			return this->ref;
		}
		
		static std::shared_ptr<Function> Create(lua_State* luaEnv, const std::string& name);

		static std::shared_ptr<Function> Create(lua_State* luaEnv, const std::string& tabName, const std::string& name);

	 public:

		template<typename Ret, typename... Args>
		Ret Func(Args... args);

		template<typename... Args>
		void Action(Args... args);



		inline int GetFunctionRef() const
		{
			return this->ref;
		}

	 public:
		static bool Get(lua_State * lua, const char * func);
		static bool Get(lua_State * lua, const char * tab, const char * func);
	 public:
        static void Clear(lua_State * lua);
        static WaitLuaTaskSource * Call(lua_State * lua);
        static WaitLuaTaskSource * Call(lua_State * lua, int ref);
        static WaitLuaTaskSource * Call(lua_State * lua, int ref, const char * func);
        static WaitLuaTaskSource * Call(lua_State * lua, const char * tab, const char * func);
	 public:
		template<typename T>
		static T Invoke(lua_State * lua);
	 private:
		int ref;
		lua_State* luaEnv;
	 private:
		static std::unordered_map<std::string, int> mRefFunctions;
	};

	template<typename... Args>
	inline void Function::Action(Args... args)
	{
		lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->ref);
		Parameter::WriteArgs<Args...>(this->luaEnv, std::forward<Args>(args)...);
		if (lua_pcall(this->luaEnv, sizeof...(Args), 0, 0) != 0)
		{
			throw std::logic_error(lua_tostring(luaEnv, -1));
		}
	}

	template<typename Ret, typename... Args>
	inline Ret Function::Func(Args... args)
	{
		lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->ref);
		Parameter::WriteArgs<Args...>(this->luaEnv, std::forward<Args>(args)...);
		if (lua_pcall(this->luaEnv, sizeof...(Args), 1, 0) != 0)
		{
			throw std::logic_error(lua_tostring(luaEnv, -1));
		}
		return Parameter::Read<Ret>(this->luaEnv, -1);
	}
	template<typename T>
	T Function::Invoke(lua_State* lua)
	{
		if (lua_pcall(lua, 0, 1, 0) != 0)
		{
			luaL_error(lua, lua_tostring(lua, -1));
		}
		return Parameter::Read<T>(lua, -1);
	}
}

