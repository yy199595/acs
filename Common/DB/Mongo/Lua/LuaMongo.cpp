#include"LuaMongo.h"
#include"Entity/Actor/App.h"
#include"Yyjson/Lua/ljson.h"
#include"Mongo/Client/MongoFactory.h"
#include"Mongo/Component/MongoDBComponent.h"

using namespace acs;
namespace Lua
{
	int LuaMongo::Run(lua_State * L)
	{
		using MongoComponent = acs::MongoDBComponent;
		static MongoComponent * component = App::Get<MongoComponent>();
		if(component == nullptr)
		{
			luaL_error(L, "not find MongoDBComponent");
			return 0;
		}
		lua_pushthread(L);
		json::r::Document document;
		const char * tab = luaL_checkstring(L, 1);
		const char * cmd = luaL_checkstring(L, 2);
		if(lua_isstring(L, 3))
		{
			size_t len = 0;
			const char * json = luaL_checklstring(L, 3, &len);
			if(!document.Decode(json, len))
			{
				luaL_error(L, "decode json fail");
				return 0;
			}
		}
		else if(lua_istable(L, 3))
		{
			std::string json;
			lua::yyjson::read(L, 3, json);
			if(!document.Decode(json.c_str(), json.size()))
			{
				luaL_error(L, "decode json fail");
				return 0;
			}
		}

		std::unique_ptr<mongo::Request> request = mongo::MongoFactory::Command(tab, cmd, document);
		if(request == nullptr)
		{
			luaL_error(L, "create mongo request fail");
			return 0;
		}
		int taskId = 0;
		component->LuaSend(std::move(request), taskId);
		return component->AddTask(new LuaMongoTask(L, taskId))->Await();
	}
}