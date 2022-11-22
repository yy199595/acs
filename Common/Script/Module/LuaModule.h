//
// Created by yjz on 2022/11/22.
//

#ifndef _LUAMODULE_H_
#define _LUAMODULE_H_
#include"Lua/LuaInclude.h"
namespace Lua
{
	class LuaModule
	{
	 public:
		LuaModule(lua_State * lua, const std::string & name, const std::string & path);
		~LuaModule();
	 public:
		bool Awake();
		bool Start();
		bool Close();
		void OnLocalComplete();
		void OnClusterComplete();
	 	bool GetFunction(const std::string & name, bool cache = true);
	 private:
		int mRef;
		lua_State * mLua;
	 	const std::string mName;
	 	const std::string mPath;
		 std::unordered_map<std::string, int> mFunctions;
	};
}

#endif //_LUAMODULE_H_
