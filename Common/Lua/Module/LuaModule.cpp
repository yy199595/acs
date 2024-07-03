//
// Created by yjz on 2022/11/22.
//
#include<fstream>
#include"LuaModule.h"
#include"Lua/Engine/Function.h"
#include"Log/Common/CommonLogDef.h"
#include "Util/Time/TimeHelper.h"

namespace Lua
{
	LuaModule::LuaModule(lua_State* lua, std::string name, int ref)
			: mLua(lua), mName(std::move(name)), mRef(ref), mIsUpdate(false)
	{
		this->InitModule();
	}

	LuaModule::~LuaModule()
	{
		if(this->mRef != 0)
		{
			luaL_unref(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		}
	}

	void LuaModule::OnModuleHotfix()
	{
		this->mCaches.clear();
		lua_pushnil(this->mLua);
		while (lua_next(this->mLua, -2) != 0)
		{
			if (lua_isfunction(this->mLua, -1))
			{
				std::string func(lua_tostring(this->mLua, -2));
				this->mCaches.insert(func);
			}
			lua_pop(this->mLua, 1);
		}
		this->mIsUpdate = this->mCaches.find("OnUpdate") != this->mCaches.end();
	}

	void LuaModule::InitModule()
	{
		this->SetMember("__name", this->mName);
		this->SetMember("__time", help::Time::NowSec());

		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);

		this->mCaches.clear();
		lua_pushnil(this->mLua);
		while (lua_next(this->mLua, -2) != 0)
		{
			if (lua_isfunction(this->mLua, -1))
			{
				std::string func(lua_tostring(this->mLua, -2));
				this->mCaches.insert(func);
			}
			lua_pop(this->mLua, 1);
		}
		this->mIsUpdate = this->mCaches.find("OnUpdate") != this->mCaches.end();
	}

	void LuaModule::SetMember(const char* key, long long value)
	{
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		{
			lua_pushinteger(this->mLua, value);
			lua_setfield(this->mLua, -2, key);
		}
	}

	void LuaModule::SetMember(const char* key, const std::string& value)
	{
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		{
			lua_pushlstring(this->mLua, value.c_str(), value.size());
			lua_setfield(this->mLua, -2, key);
		}
	}

	bool LuaModule::HasFunction(const std::string & name)
	{
		auto iter = this->mCaches.find(name);
		return iter != this->mCaches.end();
	}

	bool LuaModule::GetFunction(const std::string& name)
	{
		lua_settop(this->mLua, 0);
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		if (!lua_istable(this->mLua, -1))
		{
			return false;
		}
		lua_getfield(this->mLua, -1, name.c_str());

		if (lua_isfunction(this->mLua, -1))
		{
			lua_pushvalue(this->mLua, -2);
			return true;
		}
		return false;
	}

	bool LuaModule::GetMetaFunction(const std::string& name)
	{
		lua_settop(this->mLua, 0);
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		luaL_getmetafield(this->mLua, -1, name.c_str());
		if (lua_isfunction(this->mLua, -1))
		{
			lua_pushvalue(this->mLua, -2);
			return true;
		}
		return false;
	}

	void LuaModule::SpliteError(std::string& error)
	{
		const char * str = lua_tostring(this->mLua, -1);
		if(str == nullptr)
		{
			return;
		}
		const char * errorInfo = strrchr(str, '/');
		if(errorInfo == nullptr)
		{
			errorInfo = strrchr(str, '\\');
		}
		if(errorInfo != nullptr)
		{
			error.assign(errorInfo);
			return;
		}
		error.assign(str);
	}

	void LuaModule::OnCallError(const std::string & func)
	{
		std::string error;
		this->SpliteError(error);
		std::unique_ptr<custom::LogInfo> logInfo = std::make_unique<custom::LogInfo>();
		{
			logInfo->Level = custom::LogLevel::Error;
			logInfo->Content = fmt::format("[{}.{}] {}", this->mName, func, error);
		}
		Debug::Log(std::move(logInfo));
	}

}