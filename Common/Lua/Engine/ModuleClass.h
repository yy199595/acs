//
// Created by leyi on 2023/6/26.
//

#ifndef APP_MODULECLASS_H
#define APP_MODULECLASS_H
#include"Define.h"
#include<string>
namespace Lua
{
	class ModuleClass
	{
	public:
		explicit ModuleClass(lua_State * lua);
	public:
		void Register(const luaL_Reg & luaLib);
		void Register(const char * module, lua_CFunction func);
	private:
		lua_State * mLua;
	};
}


#endif //APP_MODULECLASS_H
