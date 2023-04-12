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
		LuaModule(lua_State* lua, std::string  name, std::string  path);
		~LuaModule();
	public:
		bool Awake();
		bool Start();
		bool Close();
		bool Hotfix();
		void Update(int tick);
		void OnLocalComplete();
		void OnClusterComplete();
		template<typename ... Args>
		bool Invoke(const std::string & func, Args && ... args);
		bool GetFunction(const std::string& name, bool cache = true);
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
	bool LuaModule::Invoke(const std::string& func, Args&& ... args)
	{
		if(!this->GetFunction(func))
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
