#include"LuaMongo.h"
#include"Entity/Actor/App.h"
#include"Yyjson/Lua/ljson.h"
#include"Mongo/Client/MongoFactory.h"
#include"Mongo/Component/MongoDBComponent.h"


namespace lua
{
	int LuaMongo::Run(lua_State* L)
	{
		using MongoComponent = acs::MongoDBComponent;
		static MongoComponent* component = acs::App::Get<MongoComponent>();
		if (component == nullptr)
		{
			luaL_error(L, "not find MongoDBComponent");
			return 0;
		}
		lua_pushthread(L);
		const char* tab = luaL_checkstring(L, 1);
		const char* cmd = luaL_checkstring(L, 2);
		std::unique_ptr<mongo::Request> mongoRequest;
		if (!mongo::MongoFactory::New(tab, cmd, mongoRequest))
		{
			luaL_error(L, "make mongo request error");
			return 0;
		}
		int top = lua_gettop(L);
		for (int index = 3; index <= top - 1; index += 2)
		{
			int valIndex = index + 1;
			const char* key = luaL_checkstring(L, index);
			switch (lua_type(L, valIndex))
			{
				case LUA_TNUMBER:
				{
					if (lua_isinteger(L, valIndex))
					{
						long long value = lua_tointeger(L, valIndex);
						if (value > std::numeric_limits<int>::max())
						{
							mongoRequest->document.Add(key, value);
						}
						else
						{
							mongoRequest->document.Add(key, (int)value);
						}
					}
					else
					{
						mongoRequest->document.Add(key, lua_tonumber(L, valIndex));
					}
					break;
				}
				case LUA_TSTRING:
				{
					size_t count = 0;
					const char* str = luaL_checklstring(L, valIndex, &count);
					mongoRequest->document.Add(key, str, count);
					break;
				}
				case LUA_TBOOLEAN:
				{
					bool value = lua_toboolean(L, valIndex);
					mongoRequest->document.Add(key, value);
					break;
				}
				case LUA_TTABLE:
				{
					size_t count = 0;
					std::unique_ptr<char> json;
					if (lua::yyjson::read(L, valIndex, json, count))
					{
						bson::w::Document document;
						document.FromByJson(json.get(), count);
						mongoRequest->document.Add(key, document);
					}
					break;
				}
				case LUA_TNIL:
				{
					mongoRequest->document.Add(key);
					break;
				}
				default:
					luaL_error(L, "unknown lua type");
					return 0;
			}
		}
		int taskId = 0;
		component->Send(mongoRequest, taskId);
		return component->AddTask(new acs::LuaMongoTask(L, taskId))->Await();
	}
}
