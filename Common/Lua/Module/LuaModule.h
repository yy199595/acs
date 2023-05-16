//
// Created by yjz on 2022/11/22.
//

#ifndef _LUAMODULE_H_
#define _LUAMODULE_H_
#include"Lua/Engine/Function.h"
#include"Entity/Component/IComponent.h"
namespace Lua
{
	class LuaModule
	{
	public:
		LuaModule(lua_State* lua, std::string name, const std::string & path);
		~LuaModule();
	public:
		bool Awake();
		void Start();
		bool Close();
		bool Hotfix();
		void OnComplete();
		void Update(int tick);
	public:
		template<typename ... Args>
		bool Call(const std::string & func, Args && ... args);
		template<typename ... Args>
		bool Await(const std::string & func, Args && ... args);
	public:
		bool GetFunction(const std::string& name);
		bool GetOnceFunction(const std::string& name);
	private:
		int mRef;
		bool mIsUpdate;
		std::string mMd5;
		lua_State* mLua;
		const std::string mName;
		const std::string mPath;
		std::unordered_map<std::string, int> mFunctions;
	};

	template<typename... Args>
	bool LuaModule::Call(const std::string& func, Args&& ... args)
	{
		if(!this->GetOnceFunction(func))
		{
			return false;
		}
		Parameter::WriteArgs<Args...>(this->mLua, std::forward<Args>(args)...);
		if (lua_pcall(this->mLua, sizeof...(Args), 0, 0) != LUA_OK)
		{
			return false;
		}
		return true;
	}

	template<typename... Args>
	bool LuaModule::Await(const std::string& func, Args&& ... args)
	{
		if(!this->GetOnceFunction(func))
		{
			return false;
		}
		WaitLuaTaskSource * taskSource = Lua::Function::Call(this->mLua, std::forward<Args>(args)...);
		if(taskSource != nullptr)
		{
			taskSource->Await<void>();
			return true;
		}
		return false;
	}
}

#endif //_LUAMODULE_H_
