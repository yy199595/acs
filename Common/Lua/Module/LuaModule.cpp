//
// Created by yjz on 2022/11/22.
//
#include<fstream>
#include"LuaModule.h"
#include"Lua/Engine/Function.h"
#include"Util/Md5/MD5.h"
#include"Util/String/StringHelper.h"
#include"Log/Common/CommonLogDef.h"
namespace Lua
{
	LuaModule::LuaModule(lua_State* lua, std::string  name)
		: mLua(lua), mName(std::move(name))
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

	bool LuaModule::LoadFromPath(const std::string& path)
	{
		if(!this->mMd5.empty())
		{
			return true;
		}
		
		if(luaL_dofile(this->mLua, path.c_str()) != LUA_OK)
		{
			LOG_ERROR(lua_tostring(this->mLua, -1));
			return false;
		}
		if(!lua_istable(this->mLua, -1))
		{
			LOG_FMT_ERR("load lua file : [{}.lua] return is not table", this->mName);
			return false;
		}
		std::ifstream fs;
		this->mPath = path;
		fs.open(path, std::ios::in);
		this->mMd5 = Helper::Md5::GetMd5(fs);
		this->mRef = luaL_ref(mLua, LUA_REGISTRYINDEX);
		
		this->InitModule();
		this->mIsUpdate = this->GetFunction("OnUpdate");
		return true;
	}

	void LuaModule::InitModule()
	{
		this->SetMember("__name", this->mName);
		this->SetMember("__path", this->mPath);
		this->SetMember("__time", Helper::Time::NowSecTime());
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

	void LuaModule::Update(int tick)
	{
		const static std::string name("OnUpdate");
		if (this->mIsUpdate)
		{
			this->Call(name, tick);
		}
	}

	bool LuaModule::Hotfix()
	{
		std::ifstream fs;
		fs.open(this->mPath, std::ios::in);
		std::string md5 = Helper::Md5::GetMd5(fs);
		if(this->mMd5 == md5) //文件没有改变
		{
			fs.close();
			return true;
		}
		fs.close();
		LOG_DEBUG("reload lua file : " << this->mName << ".lua");
		if(luaL_dofile(this->mLua, this->mPath.c_str()) != LUA_OK)
		{
			this->OnCallError("DoFile");
			return false;
		}
		if(!lua_istable(this->mLua, -1))
		{
			LOG_FMT_ERR("load lua file : [{}.lua] return is not table", this->mName);
			return false;
		}
		lua_unref(this->mLua, this->mRef);
		this->mRef = luaL_ref(mLua, LUA_REGISTRYINDEX);
		LOG_INFO("reload [" << this->mPath << "] successful");
		{
			this->mMd5 = md5;
			this->InitModule();
			this->mCaches.clear();
			this->mIsUpdate = this->GetFunction("OnUpdate");
		}
		return true;
	}

	bool LuaModule::AddCache(const std::string& name)
	{
		if(!this->GetFunction(name))
		{
			return false;
		}
		this->mCaches.insert(name);
		return true;
	}

	bool LuaModule::HasFunction(const std::string & name)
	{
		auto iter = this->mCaches.find(name);
		return iter != this->mCaches.end();
	}

	bool LuaModule::GetFunction(const std::string& name)
	{
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		if (!lua_istable(this->mLua, -1))
		{
			return false;
		}
		if (lua_getfield(this->mLua, -1, name.c_str()))
		{
			if(lua_isfunction(this->mLua, -1))
			{
				lua_pushvalue(this->mLua, -2);
				return true;
			}
		}
		return false;
	}

	void LuaModule::OnCallError(const std::string & func)
	{
		size_t length = 0;
		const char * errorInfo = nullptr;
		const char * str = luaL_tolstring(this->mLua, -1, &length);
		for (size_t index = length - 1; index > 0; index--)
		{
			if(str[index] == '/'|| str[index] == '\\')
			{
				errorInfo = str + index + 1;
				length = length - index - 1;
				break;
			}
		}
		if(errorInfo != nullptr && length > 0)
		{
			Debug::Log(Debug::Level::err, std::string(errorInfo, length));
			return;
		}
        LOG_FMT_ERR("[{0}.{1}] : {2}", this->mName, func, std::string(str, length));
	}

}