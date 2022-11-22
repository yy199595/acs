//
// Created by yjz on 2022/11/22.
//

#include"LuaModule.h"
#include"Lua/Function.h"
#include"Log/CommonLogDef.h"
namespace Lua
{
	LuaModule::LuaModule(lua_State* lua, const std::string& name, const std::string& path)
		: mLua(lua), mName(name), mPath(path)
	{
		this->mRef = 0;
	}

	LuaModule::~LuaModule()
	{
		if(this->mRef != 0)
		{
			luaL_unref(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		}
	}

	bool LuaModule::Awake()
	{
		if(this->mRef != 0)
		{
			return true;
		}
		if(luaL_dofile(this->mLua, this->mPath.c_str()) != LUA_OK)
		{
			LOG_ERROR(lua_tostring(this->mLua, -1));
			return false;
		}
		if(!lua_istable(this->mLua, -1))
		{
			return false;
		}
		this->mRef = luaL_ref(mLua, LUA_REGISTRYINDEX);
		if(this->GetFunction("Awake", false))
		{
			if(lua_pcall(this->mLua, 0, 1, 0) != LUA_OK)
			{
				LOG_ERROR(lua_tostring(this->mLua, -1));
				return false;
			}
		}
		return true;
	}

	bool LuaModule::Start()
	{
		if(this->GetFunction("Start", false))
		{
			WaitLuaTaskSource * luaTaskSource = Lua::Function::Call(this->mLua);
			return luaTaskSource != nullptr && luaTaskSource->Await<bool>();
		}
		return true;
	}

	void LuaModule::OnLocalComplete()
	{
		if (this->GetFunction("OnLocalComplete", false))
		{
			WaitLuaTaskSource* luaTaskSource = Lua::Function::Call(this->mLua);
			if (luaTaskSource != nullptr)
			{
				luaTaskSource->Await<void>();
			}
		}
	}

	void LuaModule::OnClusterComplete()
	{
		if (this->GetFunction("OnClusterComplete", false))
		{
			WaitLuaTaskSource* luaTaskSource = Lua::Function::Call(this->mLua);
			if (luaTaskSource != nullptr)
			{
				luaTaskSource->Await<void>();
			}
		}
	}

	bool LuaModule::Close()
	{
		if(this->GetFunction("Close", false))
		{
			WaitLuaTaskSource * luaTaskSource = Lua::Function::Call(this->mLua);
			return luaTaskSource != nullptr && luaTaskSource->Await<bool>();
		}
		return true;
	}

	bool LuaModule::GetFunction(const std::string& name, bool cache)
	{
		auto iter = this->mFunctions.find(name);
		if(iter != this->mFunctions.end())
		{
			int ref = iter->second;
			lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, ref);
			return true;
		}
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		if(!lua_istable(this->mLua, -1))
		{
			return false;
		}
		if(lua_getfield(this->mLua, -1, name.c_str()))
		{
			if(!lua_isfunction(this->mLua, -1))
			{
				return false;
			}
			if(cache)
			{
				int ref = luaL_ref(mLua, LUA_REGISTRYINDEX);
				this->mFunctions.emplace(name, ref);
				lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, ref);
			}
			return true;
		}
		return false;
	}
}