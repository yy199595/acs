//
// Created by yjz on 2022/11/22.
//

#ifndef _LUAMODULE_H_
#define _LUAMODULE_H_
#include"Lua/LuaInclude.h"
#include"Component/IComponent.h"
namespace Lua
{
	class LuaModule
	{
	public:
		LuaModule(lua_State* lua, const std::string& name, const std::string& path);
		~LuaModule();
	public:
		bool Awake();
		bool Start();
		bool Close();
		bool Hotfix();
		void Update(int tick);
		void OnLocalComplete();
		void OnClusterComplete();
		void Invoke(const std::string & func, long long user);
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
}

#endif //_LUAMODULE_H_
