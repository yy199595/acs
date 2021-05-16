#include"MysqlExtension.h"
#include<Manager/MysqlManager.h>
#include<MysqlClient/MysqlLuaTask.h>

int MysqlClient::InvokeCommand(lua_State * lua)
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
