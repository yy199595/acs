//
// Created by yjz on 2023/3/24.
//



#include"LuaMysql.h"
#include"Entity/Actor/App.h"
#include"Mysql/Lua/LuaMysqlTask.h"
#include"Proto/Include/Message.h"
#include"Mysql/Component/MysqlDBComponent.h"
#include"Proto/Component/ProtoComponent.h"
using namespace acs;
namespace lua
{
	int lmysql::Run(lua_State* L)
	{
		static MysqlDBComponent* mysql = nullptr;
		if(mysql == nullptr)
		{
			mysql = App::Get<MysqlDBComponent>();
			if(mysql == nullptr)
			{
				luaL_error(L, "not find [MysqlDBComponent]");
				return 0;
			}
		}
		int rpcId = 0;
		size_t size = 0;
		lua_pushthread(L);
		const char * sql = luaL_checklstring(L, 1, &size);
		std::unique_ptr<mysql::Request> request = std::make_unique<mysql::Request>(sql, size);
		{
			mysql->Send(std::move(request), rpcId);
		}
		return mysql->AddTask(new LuaMysqlTask(L, rpcId))->Await();
	}
}
