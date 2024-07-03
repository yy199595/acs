//
// Created by leyi on 2023/5/25.
//

#ifndef APP_LUAACTOR_H
#define APP_LUAACTOR_H
struct lua_State;
namespace joke
{
	namespace LuaActor
	{
		extern int Send(lua_State * l);
		extern int Call(lua_State * l);
		extern int Random(lua_State * l);
		extern int NewGuid(lua_State * l);
		extern int NewUuid(lua_State * l);
		extern int GetPath(lua_State * l);
		extern int GetListen(lua_State * l);
		extern int GetServers(lua_State * l);
		extern int GetConfig(lua_State * l);
		extern int HasComponent(lua_State *l);
		extern int AddComponent(lua_State * l);
		extern int MakeServer(lua_State * l);
		extern int LuaPushCode(lua_State * l, int code);
	};

	namespace LuaPlayer
	{
		extern int AddAddr(lua_State * l);
	}
}


#endif //APP_LUAACTOR_H
