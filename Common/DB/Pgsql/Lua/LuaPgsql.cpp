//
// Created by yjz on 2023/3/24.
//



#include"LuaPgsql.h"
#include"Entity/Actor/App.h"
#include"Pgsql/Lua/LuaPgsqlTask.h"
#include"Pgsql/Component/PgsqlDBComponent.h"
using namespace acs;
namespace lua
{
	int lpgsql::Run(lua_State* L)
	{
		static PgsqlDBComponent* pgsql = nullptr;
		if(pgsql == nullptr)
		{
			pgsql = App::Get<PgsqlDBComponent>();
			if(pgsql == nullptr)
			{
				luaL_error(L, "not find [MysqlDBComponent]");
				return 0;
			}
		}
		int rpcId = 0;
		size_t size = 0;
		lua_pushthread(L);
		const char * sql = luaL_checklstring(L, 1, &size);
		std::unique_ptr<pgsql::Request> request = std::make_unique<pgsql::Request>(std::string(sql, size));
		{
			pgsql->Send(std::move(request), rpcId);
		}
		return pgsql->AddTask(new LuaPgsqlTask(L, rpcId))->Await();
	}
}
