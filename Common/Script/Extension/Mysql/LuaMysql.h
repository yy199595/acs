//
// Created by zmhy0073 on 2022/6/6.
//

#ifndef SERVER_LUAMYSQL_H
#define SERVER_LUAMYSQL_H
#include<Script/LuaInclude.h>
namespace Lua
{
    namespace Mysql
    {
        int Add(lua_State * lua);
        int Delete(lua_State * lua);
        int Save(lua_State * lua);
        int Query(lua_State * lua);
        int Update(lua_State * lua);
        int QueryOnce(lua_State * lua);
    }
}


#endif //SERVER_LUAMYSQL_H
