//
// Created by zmhy0073 on 2022/9/21.
//

#ifndef APP_LUAMD5_H
#define APP_LUAMD5_H
#include"Lua/Engine/Define.h"
namespace Lua
{
    namespace md5
    {
        int ToString(lua_State *lua);
    }

	namespace base64
	{
		extern int Encode(lua_State * l);
		extern int Decode(lua_State * l);
	}
}

#endif //APP_LUAMD5_H
