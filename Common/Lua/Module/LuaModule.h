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
		LuaModule(lua_State* lua, std::string name);
		~LuaModule();
	public:
		bool Hotfix();
		void Update(int tick);
		bool LoadFromPath(const std::string & path);
	public:
		template<typename ... Args>
		int Call(const std::string & func, Args && ... args);
		template<typename ... Args>
		int Await(const std::string & func, Args && ... args);
	public:
		bool AddCache(const std::string & name);
		bool GetFunction(const std::string& name);
		bool HasFunction(const std::string & name);
	public:
		lua_State * GetLuaEnv() { return this->mLua;}
		void SetMember(const char* key, long long value);
		void SetMember(const char* key, const std::string & value);
	private:
		void InitModule();
		void OnCallError(const std::string & func);
	private:
		int mRef;
		bool mIsUpdate;
		std::string mMd5;
		lua_State* mLua;
		std::string mPath;
		const std::string mName;
		std::unordered_set<std::string> mCaches;
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
		return XCode::Successful;
	}

	template<typename... Args>
	int LuaModule::Await(const std::string& func, Args&& ... args)
	{
        Lua::Function::Get(this->mLua, "coroutine", "call");
        lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
        lua_pushlstring(this->mLua, func.c_str(), func.size());
        Lua::Parameter::WriteArgs(this->mLua, std::forward<Args>(args)...);

		if(lua_pcall(this->mLua, sizeof...(Args) + 2, 1, 0) != LUA_OK)
		{
			this->OnCallError(func);
			return XCode::CallLuaFunctionFail;
		}
		WaitLuaTaskSource * taskSource = PtrProxy<WaitLuaTaskSource>::Read(this->mLua, -1);
		if(taskSource == nullptr)
		{
			return XCode::CallLuaFunctionFail;
		}
		taskSource->Await<void>();
		return XCode::Successful;
	}
}
