#include "NetLuaRetAction.h"

namespace Sentry
{
	NetLuaRetAction * NetLuaRetAction::Create(lua_State * lua, int index)
	{
		if (lua_isfunction(lua, index))
		{
			int ref = luaL_ref(lua, LUA_REGISTRYINDEX);
			return new NetLuaRetAction(lua, ref);
		}
		return nullptr;
	}

	void NetLuaRetAction::Inovke(XCode code)
	{
		lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->ref);
		if (lua_isfunction(this->luaEnv, -1))
		{
			lua_pushinteger(this->luaEnv, (int)code);
			if (lua_pcall(this->luaEnv, 1, 0, 0) != 0)
			{
				const char * error = lua_tostring(this->luaEnv, -1);
				SayNoDebugError(error);
			}
		}
	}

	void NetLuaRetAction::Inovke(XCode code, const Message * message)
	{
		lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->ref);
		if (lua_isfunction(this->luaEnv, -1))
		{
			lua_pushinteger(this->luaEnv, (int)code);
			LuaParameter::Write<const Message *>(this->luaEnv, message);
			if (lua_pcall(this->luaEnv, 2, 0, 0) != 0)
			{
				const char * error = lua_tostring(this->luaEnv, -1);
				SayNoDebugError(error);
			}
		}
	}

	void NetLuaRetAction::Inovke(XCode code, const std::string & message)
	{
		lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->ref);
		if (lua_isfunction(this->luaEnv, -1))
		{
			lua_pushinteger(this->luaEnv, (int)code);
			if (lua_getfunction(this->luaEnv, "JsonUtil", "ToObject"))
			{
				lua_pushlstring(this->luaEnv, message.c_str(), message.size());
				if (lua_pcall(this->luaEnv, 1, 1, 0) != 0)
				{
					SayNoDebugError(lua_tostring(this->luaEnv, -1));
				}
			}
			if (lua_pcall(this->luaEnv, 2, 0, 0) != 0)
			{
				const char * error = lua_tostring(this->luaEnv, -1);
				SayNoDebugError(error);
			}
		}
	}
}

namespace Sentry
{
	NetLuaWaitAction * NetLuaWaitAction::Create(lua_State * lua, int index)
	{
		if (lua_isthread(lua, index))
		{
			int ref = luaL_ref(lua, LUA_REGISTRYINDEX);
			return new NetLuaWaitAction(lua, ref);
		}
		return nullptr;
	}

	void NetLuaWaitAction::Inovke(XCode code)
	{
		lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->ref);
		if (lua_isthread(this->luaEnv, -1))
		{
			lua_State * coroutine = lua_tothread(this->luaEnv, -1);
			lua_pushinteger(coroutine, (int)code);
			lua_resume(coroutine, luaEnv, 1);
		}
	}

	void NetLuaWaitAction::Inovke(XCode code, const Message * message)
	{
		lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->ref);
		if (lua_isthread(this->luaEnv, -1))
		{
			lua_State * coroutine = lua_tothread(this->luaEnv, -1);
			lua_pushinteger(coroutine, (int)code);
			LuaParameter::Write<const Message *>(coroutine, message);
			lua_resume(coroutine, luaEnv, 2);
		}
	}

	void NetLuaWaitAction::Inovke(XCode code, const std::string & message)
	{
		lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->ref);
		if (lua_isthread(this->luaEnv, -1))
		{
			lua_State * coroutine = lua_tothread(this->luaEnv, -1);
			lua_pushinteger(coroutine, (int)code);
			if (lua_getfunction(this->luaEnv, "JsonUtil", "ToObject"))
			{
				lua_pushlstring(this->luaEnv, message.c_str(), message.size());
				if (lua_pcall(this->luaEnv, 1, 1, 0) != 0)
				{
					const char * error = lua_tostring(this->luaEnv, -1);
				}
				if (lua_istable(this->luaEnv, -1))
				{
					lua_xmove(this->luaEnv, coroutine, 1);
				}
			}
			lua_resume(coroutine, luaEnv, 2);
		}
	}
}