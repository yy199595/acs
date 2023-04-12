#include"Function.h"
#include"Log/Common/CommonLogDef.h"
namespace Lua
{
	std::unordered_map<std::string, int> Function::mRefFunctions;
	Function::Function(lua_State* luaEnv, int ref)
	{
		this->luaEnv = luaEnv;
		this->ref = ref;
	}

	std::shared_ptr<Function> Function::Create(lua_State* luaEnv, const std::string& name)
	{
		lua_getglobal(luaEnv, name.c_str());
		if (lua_isfunction(luaEnv, -1))
		{
			int ref = luaL_ref(luaEnv, LUA_REGISTRYINDEX);
			return std::make_shared<Function>(luaEnv, ref);
		}
		return nullptr;
	}

	std::shared_ptr<Function>
	Function::Create(lua_State* luaEnv, const std::string& tabName, const std::string& name)
	{
		if (lua_getfunction(luaEnv, tabName.c_str(), name.c_str()))
		{
			int ref = luaL_ref(luaEnv, LUA_REGISTRYINDEX);
			return std::make_shared<Function>(luaEnv, ref);
		}
		return nullptr;
	}

	bool Function::Get(lua_State * lua, const char* tab, const char* func)
	{
		std::string name = fmt::format("{0}.{1}", tab, func);
		auto iter = mRefFunctions.find(name);
		if(iter != mRefFunctions.end())
		{
			int ref = iter->second;
			lua_rawgeti(lua, LUA_REGISTRYINDEX, ref);
			return lua_isfunction(lua, -1);
		}
		if(lua_getfunction(lua, tab, func))
		{
			int ref = luaL_ref(lua, LUA_REGISTRYINDEX);
			lua_rawgeti(lua, LUA_REGISTRYINDEX, ref);
			mRefFunctions.emplace(name, ref);
			return true;
		}
		return false;
	}

	bool Function::Get(lua_State* lua, const char* func)
	{
		if(lua_getglobal(lua, func))
		{
			if(lua_isfunction(lua, -1))
			{
				int ref = luaL_ref(lua, LUA_REGISTRYINDEX);
				lua_rawgeti(lua, LUA_REGISTRYINDEX, ref);
				mRefFunctions.emplace(func, ref);
				return true;
			}
		}
		return false;
	}

    WaitLuaTaskSource *Function::Call(lua_State *lua)
    {
        if(!lua_isfunction(lua, -1))
        {
            return nullptr;
        }
        if(!Lua::Function::Get(lua, "coroutine", "call"))
        {
            return nullptr;
        }
        lua_pushvalue(lua, -2);
        if (lua_pcall(lua, 1, 1, 0) != 0)
        {
            LOG_ERROR(lua_tostring(lua, -1));
            return nullptr;
        }
        return PtrProxy<WaitLuaTaskSource>::Read(lua, -1);
    }

    void Function::Clear(lua_State * lua)
    {
        auto iter = mRefFunctions.begin();
        for(; iter != mRefFunctions.end(); iter++)
        {
            lua_unref(lua, iter->second);
        }
        mRefFunctions.clear();
    }

	WaitLuaTaskSource* Function::Call(lua_State * lua, int ref)
	{
		if(!Lua::Function::Get(lua, "coroutine", "call"))
		{
			return nullptr;
		}
		lua_rawgeti(lua, LUA_REGISTRYINDEX, ref);
		if (!lua_isfunction(lua, -1))
		{
			return nullptr;
		}
		if (lua_pcall(lua, 1, 1, 0) != 0)
		{
			LOG_ERROR(lua_tostring(lua, -1));
			return nullptr;
		}
		return PtrProxy<WaitLuaTaskSource>::Read(lua, -1);
	}

    WaitLuaTaskSource *Function::Call(lua_State *lua, int ref, const char *func)
    {
        if(!Lua::Function::Get(lua, "coroutine", "call"))
        {
            return nullptr;
        }
        lua_rawgeti(lua, LUA_REGISTRYINDEX, ref);
        if (!lua_istable(lua, -1))
        {
            return nullptr;
        }
        if(!lua_getfield(lua, -1, func))
        {
            return nullptr;
        }
        lua_remove(lua, -2);
        if (lua_pcall(lua, 1, 1, 0) != 0)
        {
            LOG_ERROR(lua_tostring(lua, -1));
            return nullptr;
        }
        return PtrProxy<WaitLuaTaskSource>::Read(lua, -1);
    }

	WaitLuaTaskSource* Function::Call(lua_State* lua, const char* tab, const char* func)
	{
		if(!Lua::Function::Get(lua, "coroutine", "call"))
		{
			return nullptr;
		}
		if(!lua_getfunction(lua, tab, func))
		{
			return nullptr;
		}
		if (lua_pcall(lua, 1, 1, 0) != 0)
		{
			LOG_ERROR(lua_tostring(lua, -1));
			return nullptr;
		}
		return PtrProxy<WaitLuaTaskSource>::Read(lua, -1);
	}
}
