//
// Created by 64658 on 2024/9/3.
//

#ifndef APP_VALUEPROXY_H
#define APP_VALUEPROXY_H
#include "LuaParameter.h"
namespace Lua
{
	template<typename T, typename V>
	struct ValueProxy
	{
		static int Get(lua_State * luaEnv)
		{
			typedef V T::*Member;
			T* ptr = PtrProxy<T>::Read(luaEnv, 1);
			Member* field = (Member*)lua_touserdata(luaEnv, lua_upvalueindex(1));
			V value = ptr->*(*field);
			Lua::Parameter::Write(luaEnv, value);
			return 1;
		}
	};
}


#endif //APP_VALUEPROXY_H
