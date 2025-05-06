//
// Created by yjz on 2022/11/22.
//

#pragma once
#include"XCode/XCode.h"
#include"Lua/Engine/Function.h"
#include"Entity/Component/IComponent.h"
namespace Lua
{
	class LuaModule
	{
	public:
		LuaModule(lua_State* lua, std::string name, int ref);
		~LuaModule();
	public:
		void OnModuleHotfix();
		const std::string & Name() const { return this->mName; }
	public:
		template<typename ... Args>
		int Call(const std::string & func, Args && ... args);
		template<typename ... Args>
		int Await(const std::string & func, Args && ... args);
	public:
		bool GetFunction(const std::string& name);
		bool HasFunction(const std::string & name);
		bool GetMetaFunction(const std::string & name) noexcept;
	public:
		void SplitError(std::string & error);
		void SetMember(const char* key, long long value);
		inline lua_State * GetLuaEnv() { return this->mLua;}
		void SetMember(const char* key, const std::string & value);
	private:
		void InitModule();
		void OnCallError(const std::string & func);
	private:
		int mRef;
		lua_State* mLua;
		const std::string mName;
		std::vector<std::string> mCaches;
	};

	template<typename... Args>
	int LuaModule::Call(const std::string& func, Args&& ... args)
	{
		if(!this->GetFunction(func))
		{
			return XCode::CallFunctionNotExist;
		}
		Parameter::WriteArgs<Args...>(this->mLua, std::forward<Args>(args)...);
		if (lua_pcall(this->mLua, sizeof...(Args) + 1, 0, 0) != LUA_OK)
		{
			this->OnCallError(func);
			return XCode::CallLuaFunctionFail;
		}
		return XCode::Ok;
	}

	template<typename... Args>
	int LuaModule::Await(const std::string& func, Args&& ... args)
	{
		lua_settop(this->mLua, 0);
		if(!this->HasFunction(func))
		{
			return XCode::CallFunctionNotExist;
		}
		this->GetMetaFunction("Await");
		lua_pushstring(this->mLua, func.c_str());
		std::unique_ptr<WaitLuaTaskSource> task = std::make_unique<WaitLuaTaskSource>();
		{
			Lua::Parameter::Write<WaitLuaTaskSource*>(this->mLua, task.get());
			Lua::Parameter::WriteArgs(this->mLua, std::forward<Args>(args)...);
		}
		if(lua_pcall(this->mLua, sizeof...(Args) + 3, 1, 0) != LUA_OK)
		{
			this->OnCallError(func);
			return XCode::CallLuaFunctionFail;
		}
		if(!lua_toboolean(this->mLua, -1))
		{
			return XCode::Ok;
		}
		task->Await<void>();
		return XCode::Ok;
	}
}
