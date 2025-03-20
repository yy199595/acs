//
// Created by leyi on 2023/5/25.
//

#ifndef APP_LUAACTOR_H
#define APP_LUAACTOR_H
struct lua_State;
namespace acs
{
	namespace LuaActor
	{
		extern int Stop(lua_State * l);
		extern int Send(lua_State * l);
		extern int Call(lua_State * l);
		extern int Publish(lua_State * l);
		extern int NewGuid(lua_State * l);
		extern int NewUuid(lua_State * l);
		extern int GetPath(lua_State * l);
		extern int GetListen(lua_State * l);
		extern int AddListen(lua_State * l);
		extern int GetServers(lua_State * l);
		extern int GetConfig(lua_State * l);
		extern int HasComponent(lua_State *l);
		extern int MakeServer(lua_State * l);
		extern int RemoveServer(lua_State * L);
		extern int AllInfo(lua_State * l);
		extern int LuaPushCode(lua_State * l, int code);
	};
}


#endif //APP_LUAACTOR_H
