#include"MysqlExtension.h"
#include<Manager/MysqlManager.h>
#include<MysqlClient/MysqlLuaTask.h>
#include<RedisClient/RedisLuaTask.h>
int SoEasy::InvokeMysqlCommand(lua_State * lua)
{
	size_t size;
	Applocation * app = Applocation::Get();
	MysqlManager * pMysqlManager = app->GetManager<MysqlManager>();
	const std::string db = lua_tostring(lua, 1);
	const char * sql = lua_tolstring(lua, 2, &size);
	lua_pushthread(lua);
	if (MysqlLuaTask::Start(lua, -1, db, std::string(sql, size)))
	{
		return lua_yield(lua, 1);
	}
	return 0;
}

int SoEasy::InvokeRedisCommand(lua_State * luaEnv)
{
	int n = lua_gettop(luaEnv);
	std::stringstream stringBuffer;
	std::vector<std::string> command;
	for (int i = 1; i <= n; i++)
	{
		lua_pushvalue(luaEnv, -1);
		lua_pushvalue(luaEnv, i);
		if (lua_isnumber(luaEnv, -1))
		{
			long long num = lua_tointeger(luaEnv, -1);
			command.push_back(std::to_string(num));
		}
		else if (lua_isstring(luaEnv, -1))
		{
			size_t size = 0;
			const char * str = lua_tolstring(luaEnv, -1, &size);
			command.push_back(std::string(str, size));
		}
	}
	lua_pushthread(luaEnv);
	if (RedisLuaTask::Start(luaEnv, -1, command))
	{
		return lua_yield(luaEnv, 1);
	}
	return 0;
}