//
// Created by yjz on 2022/11/22.
//

#include"LuaModule.h"
#include"Lua/Function.h"
#include"Md5/MD5.h"
#include<fstream>
#include"File/FileHelper.h"
#include"Log/CommonLogDef.h"
namespace Lua
{
	LuaModule::LuaModule(lua_State* lua, const std::string& name, const std::string& path)
		: mLua(lua), mName(name), mPath(path)
	{
		this->mRef = 0;
		this->mIsUpdate = false;
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
		if(!this->mMd5.empty())
		{
			return true;
		}
		if(luaL_dofile(this->mLua, this->mPath.c_str()) != LUA_OK)
		{
            LOG_FATAL(lua_tostring(this->mLua, -1));
			return false;
		}
		if(!lua_istable(this->mLua, -1))
		{
			LOG_ERROR(this->mName << " is not return lua table");
			return false;
		}
		std::ifstream fs(this->mPath, std::ios::in);
		MD5 md5(fs);
		this->mMd5 = md5.toString();
		this->mRef = luaL_ref(mLua, LUA_REGISTRYINDEX);
		if(this->GetFunction("Awake", false))
		{
			if(lua_pcall(this->mLua, 0, 1, 0) != LUA_OK)
			{
                LUA_LOG_ERROR(lua_tostring(this->mLua, -1));
				return false;
			}
		}
		this->mIsUpdate = this->GetFunction("Update");
		return true;
	}

	void LuaModule::Update(int tick)
	{
		const static std::string name("Update");
		if (this->mIsUpdate && this->GetFunction(name))
		{
			lua_pushinteger(this->mLua, tick);
			if (lua_pcall(this->mLua, 1, 0, 0) != LUA_OK)
			{
                LUA_LOG_ERROR(lua_tostring(this->mLua, -1));
			}
		}
	}

	bool LuaModule::Hotfix()
	{
		std::ifstream fs(this->mPath, std::ios::in);
		MD5 md5(fs);
		if(this->mMd5 == md5.toString()) //文件没有改变
		{
			return true;
		}
		if(luaL_dofile(this->mLua, this->mPath.c_str()) != LUA_OK)
		{
            LUA_LOG_ERROR(lua_tostring(this->mLua, -1));
			return false;
		}
		if(!lua_istable(this->mLua, -1))
		{
			return false;
		}
		int ref = this->mRef;
		std::set<std::string> functions;
		this->mRef = luaL_ref(mLua, LUA_REGISTRYINDEX);
		luaL_unref(this->mLua, LUA_REGISTRYINDEX, ref);
		auto iter = this->mFunctions.begin();
		for(; iter != this->mFunctions.end(); iter++)
		{
			ref = iter->second;
			functions.insert(iter->first);
			luaL_unref(this->mLua, LUA_REGISTRYINDEX, ref);
		}
		this->mFunctions.clear();
		this->mMd5 = md5.toString();
		for(const std::string & func : functions)
		{
			this->GetFunction(func);
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