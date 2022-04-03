#pragma once

#include"LuaInclude.h"

namespace Lua
{
	namespace UserDataParameter
	{
		template<typename T>
		struct UserDataStruct
		{
			static T Read(lua_State* lua, int index)
			{
				return T();
			}

			static void Write(lua_State* lua, T data)
			{
				assert(false);
			}
		};

		template<typename T>
		struct UserDataStruct<T&>
		{
			static T& Read(lua_State* lua, int index)
			{
				return T();
			}

			static void Write(lua_State* lua, T& data)
			{
				size_t size = sizeof(PtrProxy<T>);
				new(lua_newuserdata(lua, size))PtrProxy<T>(&data);
				lua_getglobal(lua, ClassNameProxy::GetLuaClassName<T>());
				if (lua_istable(lua, -1))
				{
					lua_setmetatable(lua, -2);
				}
			}
		};
	}
	namespace UserDataParameter
	{
		template<typename T>
		struct UserDataStruct<T*>
		{
			static T* Read(lua_State* lua, int index)
			{
				return PtrProxy<T>::Read(lua, index);
			}

			static void Write(lua_State* lua, T* data)
			{
				const char* typeName = ClassNameProxy::GetLuaClassName<T>();
				if (data != nullptr && typeName != nullptr)
				{
					size_t size = sizeof(PtrProxy<T>);
					new(lua_newuserdata(lua, size))PtrProxy<T>(data);
					lua_getglobal(lua, typeName);
					if (lua_istable(lua, -1))
					{
						lua_setmetatable(lua, -2);
						return;
					}
				}
				lua_pushnil(lua);
			}
		};
	}
	using namespace std;
	namespace UserDataParameter
	{
		template<typename T>
		struct UserDataStruct<shared_ptr<T>>
		{
			static shared_ptr<T> Read(lua_State* lua, int index)
			{
				return SharedPtrProxy<T>::Read(lua, index);
			}

			static void Write(lua_State* lua, shared_ptr<T> data)
			{
				const char* typeName = ClassNameProxy::GetLuaClassName<T>();
				if (data != nullptr && typeName != nullptr)
				{
					size_t size = sizeof(SharedPtrProxy<T>);
					new(lua_newuserdata(lua, size))SharedPtrProxy<T>(data);
					lua_getglobal(lua, typeName);
					if (lua_istable(lua, -1))
					{
						lua_setmetatable(lua, -2);
						return;
					}
				}
				lua_pushnil(lua);
			}
		};
	}

	namespace UserDataParameter
	{
		template<typename T>
		inline T Read(lua_State* lua, int index)
		{
			return UserDataStruct<T>::Read(lua, index);
		}

		template<typename T>
		inline void Write(lua_State* lua, T data)
		{
			UserDataStruct<T>::Write(lua, data);
		}
	}
}