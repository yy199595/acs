#include"MysqlExtension.h"
#include<Manager/MysqlManager.h>
#include<Manager/RedisManager.h>
#include<MysqlClient/MysqlLuaTask.h>
#include<RedisClient/RedisLuaTask.h>
int SoEasy::InvokeMysqlCommand(lua_State * lua)
{
	size_t size;
	Applocation * app = Applocation::Get();
	MysqlManager * pMysqlManager = app->GetManager<MysqlManager>();
	if (!lua_isstring(lua, 1))
	{
		lua_pushnil(lua);
		return 1;
	}
	if (!lua_isstring(lua, 2))
	{
		lua_pushnil(lua);
		return 1;
	}
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
	if (!lua_isstring(luaEnv, 1))
	{
		lua_pushnil(luaEnv);
		return 1;
	}
	Applocation * app = Applocation::Get();
	RedisManager * redisManager = app->GetManager<RedisManager>();

	const char * cmd = lua_tostring(luaEnv, 1);
	lua_pushthread(luaEnv);
	shared_ptr<RedisLuaTask> taskAction = RedisLuaTask::Create(luaEnv, -1, cmd);
	if (taskAction == nullptr)
	{
		lua_pushnil(luaEnv);
		return 1;
	}
	for (int i = 2; i <= n; i++)
	{
		lua_pushvalue(luaEnv, -1);
		lua_pushvalue(luaEnv, i);
		if (lua_isnumber(luaEnv, -1))
		{
			long long num = lua_tointeger(luaEnv, -1);
			taskAction->AddCommandArgv(std::to_string(num));
		}
		else if (lua_isstring(luaEnv, -1))
		{
			size_t size = 0;
			const char * str = lua_tolstring(luaEnv, -1, &size);
			taskAction->AddCommandArgv(str, size);
		}
	}
	if (redisManager->StartTaskAction(taskAction))
	{
		return lua_yield(luaEnv, 1);
	}
	return 0;
}