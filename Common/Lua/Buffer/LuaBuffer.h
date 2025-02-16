//
// Created by 64658 on 2025/1/17.
//

#ifndef APP_LUABUFFER_H
#define APP_LUABUFFER_H
#include <string>
#include <type_traits>
#include "Lua/Engine/Define.h"
namespace lua
{
	struct Buffer
	{
		explicit Buffer(lua_State * l);

		Buffer & operator << (const char * str)
		{
			luaL_addstring(&this->B, str);
		}
		Buffer & operator << (const std::string & str)
		{
			luaL_addlstring(&this->B, str.c_str(), str.size());
		}

		template<typename T>
		std::enable_if_t<std::is_integral<T>::value, Buffer&> operator << (const T & value)
		{
			std::string str = std::to_string(value);
			luaL_addlstring(&this->B, str.c_str(), str.size());
		}
		template<typename T>
		std::enable_if_t<std::is_floating_point<T>::value, Buffer&> operator << (const T & value)
		{
			std::string str = std::to_string(value);
			luaL_addlstring(&this->B, str.c_str(), str.size());
		}
		inline int PushValue() { luaL_pushresult(&this->B); return 1; }

	public:
		lua_State * L;
		luaL_Buffer B;
	};

	Buffer::Buffer(lua_State* l)
		: L(l)
	{
		luaL_buffinit(l, &this->B);
	}
}
#endif //APP_LUABUFFER_H
