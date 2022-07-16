#include"Util.h"
#include"Util/MD5.h"
#include"Util/sha1.h"
namespace Lua
{
	int Md5::ToString(lua_State * lua)
	{
		size_t size = 0;
		MD5 md5(luaL_checklstring(lua, -1, &size), size);
		std::string str = md5.toString();
		lua_pushlstring(lua, str.c_str(), str.size());
		return 1;
	}

	int Sha1::GetHash(lua_State* lua)
	{
		size_t size1 = 0;
		const char* text = luaL_checklstring(lua, 1, &size1);
		std::string hash = Helper::Sha1::GetHash(text, size1);
		lua_pushlstring(lua, text, size1);
		return 1;
	}

	int Sha1::GetMacHash(lua_State* lua)
	{
		size_t size1, size2 = 0;
		const char* key = luaL_checklstring(lua, 1, &size1);
		const char* text = luaL_checklstring(lua, 2, &size2);
		std::string hash = Helper::Sha1::GetHMacHash(std::string(key, size1), std::string(text, size2));
		lua_pushlstring(lua, text, size1);
		return 1;
	}
}