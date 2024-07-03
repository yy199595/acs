//
// Created by yy on 2023/8/19.
//

#ifndef APP_LUATABLE_H
#define APP_LUATABLE_H
#include"Lua/Engine/LuaInclude.h"
#include"Proto/Include/Message.h"
namespace lua
{
	class LuaTable
	{
	public:
		explicit LuaTable(lua_State * lua, int count = 0);
	public:
		void SetMember(const char * key, int value);
		void SetMember(const char * key, long long value);
		void SetMember(const char * key, const pb::Message & value);
		void SetMember(const char * key, const std::string & value, bool json = false);
	private:
		lua_State * mLua;
	};
}

#endif //APP_LUATABLE_H
