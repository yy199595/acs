//
// Created by zmhy0073 on 2022/9/21.
//

#include"LuaMd5.h"
#include"MD5.h"
#include<string>
namespace Lua
{
    int Md5::ToString(lua_State *lua)
    {
        size_t size = 0;
        MD5 md5(luaL_checklstring(lua, -1, &size), size);
        const std::string str = md5.toString();
        lua_pushlstring(lua, str.c_str(), str.size());
        return 1;
    }
}
