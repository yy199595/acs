#include "LuaInclude.h"

namespace Lua
{
	namespace ClassMateProxy
	{
		template<typename T>
		inline int OnMateTableGet(lua_State* lua)
		{
			const char* field = lua_tostring(lua, 2);
			if (lua_getmetatable(lua, 1) != 0 && lua_istable(lua, -1))
			{
				lua_getfield(lua, -1, field);
				if (!lua_isnil(lua, -1))
				{
					return 1;
				}
			}

			if (lua_isnil(lua, -1))
			{
				lua_pop(lua, 1);
				std::vector<std::string>* parents = ClassNameProxy::GetParents<T>();
				if (parents != nullptr && !parents->empty())
				{
					for (size_t index = 0; index < parents->size(); index++)
					{
						const char* parent = parents->at(index).c_str();
						if (lua_getglobal(lua, parent) && lua_istable(lua, -1))
						{
							lua_getfield(lua, -1, field);
							if (!lua_isnil(lua, -1))
							{
								return 1;
							}
							lua_pop(lua, 1);
						}
					}
				}
			}
			return 0;
		}

		template<typename T>
		inline int OnDestory(lua_State* lua)
		{
			PtrProxy<T>::Destory(lua, -1);
			//printf("destory object %s\n", typeid(T).name());
			return 0;
		}
	}// namespace ClassMateProxy
}