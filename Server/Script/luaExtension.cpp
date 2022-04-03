#include "luaExtension.h"
#include "App/App.h"
#include "Component/Component.h"
#include "Util/StringHelper.h"
#include"Util/DirectoryHelper.h"
using namespace Sentry;

namespace Lua
{

	namespace LuaAPIExtension
	{
		int GameObjectGetComponent(lua_State* lua)
		{
			LOG_CHECK_RET_ZERO(lua_isuserdata(lua, 1));
			LOG_CHECK_RET_ZERO(lua_isstring(lua, 2));
			Entity* gameObject = PtrProxy<Entity>::Read(lua, 1);
			if (gameObject)
			{
				const char* moduleName = lua_tostring(lua, 2);
				Component* component = gameObject->GetComponent<Component>(moduleName);
				if (component)
				{
					PtrProxy<Component>::Write(lua, component);
					lua_getglobal(lua, moduleName);
					lua_setmetatable(lua, -2);
					return 1;
				}
			}
			lua_pushnil(lua);
			return 1;
		}

		int ComponentGetComponent(lua_State* lua)
		{

			return 1;
		}

		inline std::map<int, std::string> GetLuaStackData(lua_State* lua)
		{
			std::map<int, std::string> ret;

			int top = lua_gettop(lua);
			for (int index = 1; index <= top; index++)
			{
				std::string name = "";
				switch (lua_type(lua, -index))
				{
				case LUA_TNIL:
					name = "nil";
					ret.emplace(-index, "nil");
					break;
				case LUA_TBOOLEAN:
					ret.emplace(-index, "nil");
					break;
				case LUA_TLIGHTUSERDATA:
					ret.emplace(-index, "lightuserdata");
					break;
				case LUA_TNUMBER:
					ret.emplace(-index, "number");
					break;
				case LUA_TSTRING:
					ret.emplace(-index, "string");
					break;
				case LUA_TTABLE:
					ret.emplace(-index, "table");
					break;
				case LUA_TFUNCTION:
					ret.emplace(-index, "function");
					break;
				case LUA_TUSERDATA:
					ret.emplace(-index, "userdata");
					break;
				case LUA_TTHREAD:
					ret.emplace(-index, "thread");
					break;
				}
			}
			return ret;
		}

		int GetComponent(lua_State* lua)
		{
			LOG_CHECK_RET_ZERO(lua_isuserdata(lua, -2));
			LOG_CHECK_RET_ZERO(lua_isstring(lua, -1));
			const char* name = lua_tostring(lua, -1);
			Entity* gameObject = PtrProxy<Entity>::Read(lua, -2);
			if (gameObject)
			{
				Component* component = gameObject->GetComponent<Component>(name);
				if (component)
				{
					PtrProxy<Component>::Write(lua, component);
					lua_getglobal(lua, name);
					lua_setmetatable(lua, -2);
					return 1;
				}
			}
			return 0;
		}

		int AddComponent(lua_State* lua)
		{
			LOG_CHECK_RET_ZERO(lua_isuserdata(lua, -2));
			LOG_CHECK_RET_ZERO(lua_isstring(lua, -1));
			const char* name = lua_tostring(lua, -1);
			Entity* gameObject = PtrProxy<Entity>::Read(lua, -2);
			if (gameObject)
			{
				if (gameObject->AddComponent(name))
				{
					Component* component = gameObject->GetComponent<Component>(name);
					PtrProxy<Component>::Write(lua, component);
					lua_getglobal(lua, name);
					lua_setmetatable(lua, -2);
					return 1;
				}
			}
			lua_pushnil(lua);
			return 1;
		}

		int TypeCast(lua_State* luaEnv)
		{
			if (lua_isuserdata(luaEnv, 1) && lua_isstring(luaEnv, 2))
			{
				const char* name = lua_tostring(luaEnv, 2);
				lua_pop(luaEnv, 1);
				if (luaL_newmetatable(luaEnv, name) != 0)
				{
					lua_setmetatable(luaEnv, -2);
				}
				return 1;
			}
			return 0;
		}
	}// namespace LuaAPIExtension
}