//
// Created by leyi on 2023/6/26.
//

#ifndef APP_MODULECLASS_H
#define APP_MODULECLASS_H
#include"Define.h"
#include<string>
namespace Lua
{
	class CCModule
	{
	public:
		explicit CCModule(lua_State * lua);
	public:
		void Open(const luaL_Reg & luaLib);
		void Open(const char * module, lua_CFunction func);
	private:
		lua_State * mLua;
	};
}


#endif //APP_MODULECLASS_H
