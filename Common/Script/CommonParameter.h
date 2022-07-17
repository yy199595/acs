#pragma once

#include "LuaInclude.h"

namespace Lua
{
	namespace CommonParameter
	{
		template<typename T>
		inline T Read(lua_State* lua, int index)
		{
			return T();
		}

		template<typename T>
		inline void Write(lua_State* lua, T data)
		{
		}
	}// namespace CommonParameter


	namespace CommonParameter
	{
		template<> inline int Read(lua_State* lua, int index)
		{
			assert(lua_isinteger(lua, index));
			return lua_tointeger(lua, index);
		}

		template<> inline bool Read(lua_State* lua, int index)
		{
			assert(lua_isboolean(lua, index));
			return (bool)lua_toboolean(lua, index);
		}

		template<> inline unsigned int Read(lua_State* lua, int index)
		{
			assert(lua_isinteger(lua, index));
			return lua_tointeger(lua, index);
		}

		template<> inline long long Read(lua_State* lua, int index)
		{
			assert(lua_isinteger(lua, index));
			return lua_tointeger(lua, index);
		}
		template<> inline unsigned long long Read(lua_State* lua, int index)
		{
			assert(lua_isinteger(lua, index));
			return lua_tointeger(lua, index);
		}

		template<> inline float Read(lua_State* lua, int index)
		{
			assert(lua_isnumber(lua, index));
			return lua_tonumber(lua, index);
		}

		template<> inline double Read(lua_State* lua, int index)
		{
			assert(lua_isnumber(lua, index));
			return lua_tonumber(lua, index);
		}

		template<> inline const char* Read(lua_State* lua, int index)
		{
			assert(lua_isstring(lua, index));
			return lua_tostring(lua, index);
		}

		template<> inline short Read(lua_State* lua, int index)
		{
			assert(lua_isinteger(lua, index));
			return lua_tointeger(lua, index);
		}

		template<> inline unsigned short Read(lua_State* lua, int index)
		{
			assert(lua_isinteger(lua, index));
			return lua_tointeger(lua, index);
		}

		template<> inline void Read(lua_State* lua, int index)
		{
		}

		template<>
		inline std::string& Read(lua_State* lua, int index)
		{
			size_t size = 0;
			static std::string data = "";
			assert(lua_isstring(lua, index));
			const char* str = lua_tolstring(lua, index, &size);
			data.assign(str, size);
			return data;
		}

//		template<>
//		inline const std::string& Read(lua_State* lua, int index)
//		{
//			size_t size = 0;
//			static std::string str = "";
//			assert(lua_isstring(lua, index));
//			str.assign(lua_tolstring(lua, index, &size), size);
//			return str;
//		}

		template<>
		inline std::string Read(lua_State* lua, int index)
		{
			size_t size = 0;
			assert(lua_isstring(lua, index));
			const char* p = lua_tolstring(lua, index, &size);
			return std::string(p, size);
		}
	}// namespace CommonParameter

	namespace CommonParameter
	{

		template<>
		inline void Write(lua_State* lua, int data)
		{
			lua_pushinteger(lua, data);
		}

		template<>
		inline void Write(lua_State* lua, bool data)
		{
			lua_pushboolean(lua, data);
		}

		template<>
		inline void Write(lua_State* lua, unsigned int data)
		{
			lua_pushinteger(lua, data);
		}

		template<>
		inline void Write(lua_State* lua, short data)
		{
			lua_pushinteger(lua, data);
		}

		template<>
		inline void Write(lua_State* lua, unsigned short data)
		{
			lua_pushinteger(lua, data);
		}

		template<>
		inline void Write(lua_State* lua, long long data)
		{
			lua_pushinteger(lua, data);
		}

		template<>
		inline void Write(lua_State* lua, unsigned long long data)
		{
			lua_pushinteger(lua, data);
		}

		template<>
		inline void Write(lua_State* lua, float data)
		{
			lua_pushnumber(lua, data);
		}

		template<>
		inline void Write(lua_State* lua, double data)
		{
			lua_pushnumber(lua, data);
		}

		template<>
		inline void Write(lua_State* lua, char* data)
		{
			lua_pushstring(lua, data);
		}

		template<>
		inline void Write(lua_State* lua, const char* data)
		{
			lua_pushstring(lua, data);
		}

		template<>
		inline void Write(lua_State* lua, std::string data)
		{
			lua_pushlstring(lua, data.c_str(), data.length());
		}

		template<>
		inline void Write(lua_State* lua, std::string& data)
		{
			lua_pushlstring(lua, data.c_str(), data.length());
		}

		template<>
		inline void Write(lua_State* lua, const std::string& data)
		{
			lua_pushlstring(lua, data.c_str(), data.length());
		}

	}// namespace CommonParameter
}