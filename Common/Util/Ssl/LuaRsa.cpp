//
// Created by 64658 on 2024/11/7.
//
#ifdef __ENABLE_OPEN_SSL__

#include "rsa.h"
#include "LuaRsa.h"
#include "Yyjson/Lua/ljson.h"
#include "Lua/Engine/UserDataParameter.h"

namespace lua
{
	int rsa::Init(lua_State* L)
	{
		std::string pubKey = luaL_checkstring(L, 1);
		std::string priKey = luaL_checkstring(L, 2);
		std::unique_ptr<ssl::RSAEncryptor> rsaEncryptor = std::make_unique<ssl::RSAEncryptor>();
		if (!rsaEncryptor->Init(pubKey, priKey))
		{
			lua_pushnil(L);
			return 0;
		}
		Lua::UserDataParameter::Write<ssl::RSAEncryptor*>(L, rsaEncryptor.release());
		return 1;
	}

	int rsa::Encode(lua_State* L)
	{
		ssl::RSAEncryptor * rsaEncryptor = Lua::UserDataParameter::Read<ssl::RSAEncryptor*>(L, 1);
		if(rsaEncryptor == nullptr)
		{
			return 0;
		}
		std::string input;
		std::string output;
		if(lua_isstring(L, 2))
		{
			size_t size = 0;
			const char * str = luaL_checklstring(L, 2, &size);
			input.assign(str, size);
		}
		else if(lua_istable(L, 2))
		{
			lua::yyjson::read(L, 2, input);
		}
		if(!rsaEncryptor->Encode(input, output))
		{
			return 0;
		}
		lua_pushlstring(L, output.c_str(), output.size());
		return 1;
	}

	int rsa::Decode(lua_State* L)
	{
		ssl::RSAEncryptor * rsaEncryptor = Lua::UserDataParameter::Read<ssl::RSAEncryptor*>(L, 1);
		if(rsaEncryptor == nullptr)
		{
			return 0;
		}
		size_t size = 0;
		std::string output;
		const char * input = luaL_checklstring(L, 2, &size);
		if(!rsaEncryptor->Decode(std::string(input, size), output))
		{
			return 0;
		}
		lua_pushlstring(L, output.c_str(), output.size());
		return 1;
	}
}
#endif