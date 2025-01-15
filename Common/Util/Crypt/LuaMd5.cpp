//
// Created by zmhy0073 on 2022/9/21.
//


#include"md5.h"
#include<ctime>
#include"LuaMd5.h"
#include"Proto/Bson/base64.h"
#include"Util/Tools/String.h"
namespace lua
{
    int md5::ToString(lua_State *lua)
    {
		MD5 md5;
		if(lua_isstring(lua, 1))
		{
			size_t size = 0;
			const char * str = luaL_checklstring(lua, -1, &size);
			md5.update(str, size);
		}
		else
		{
			time_t t;
			time(&t);
			std::string str = help::Str::RandomString();
			md5.update(str + std::to_string(t));
		}
        const std::string str = md5.toString();
        lua_pushlstring(lua, str.c_str(), str.size());
        return 1;
    }
}

namespace lua
{
	int base64::Encode(lua_State* l)
	{
		size_t size = 0;
		const char * str = luaL_checklstring(l, 1, &size);
		const std::string target = _bson::base64::encode(str, (int)size);
		lua_pushlstring(l, target.c_str(), target.size());
		return 1;
	}

	int base64::Decode(lua_State* l)
	{
		size_t size = 0;
		const char * str = luaL_checklstring(l, 1, &size);
		const std::string target = _bson::base64::decode(std::string(str, size));
		lua_pushlstring(l, target.c_str(), target.size());
		return 1;
	}
}


namespace lua
{
	int lib::luaopen_lmd5(lua_State* L)
	{
		luaL_Reg l[] = {
				{ "ToString", lua::md5::ToString },
				{ nullptr, nullptr}
		};
		luaL_newlib(L, l);
		return 1;
	}

	int lib::luaopen_lbase64(lua_State* L)
	{
		luaL_Reg l[] = {
				{ "encode", lua::base64::Encode },
				{ "decode", lua::base64::Decode },
				{ nullptr, nullptr}
		};
		luaL_newlib(L, l);
		return 1;
	}
}