//
// Created by leyi on 2023/5/8.
//

#ifndef APP_LOCALCLASSPROXY_H
#define APP_LOCALCLASSPROXY_H
#include<string>
#include"Lua/Engine/Define.h"
#include "LuaInclude.h"

namespace Lua
{
	class LocalClassProxy
	{
	public:
		LocalClassProxy(lua_State * lua) : mLua(lua) { }
	public:
		void NewTable(const std::string & name, int count = 0);

		template<typename T>
		void PushMember(const std::string & name, const T & value);
		void PushMemberFunction(const std::string & name, lua_CFunction func);
	private:
		lua_State * mLua;
		std::string mName;
	};

	template<typename T>
	void LocalClassProxy::PushMember(const std::string& name, const T& value)
	{
		Lua::Parameter::Write<T>(this->mLua, value);
		lua_setfield(this->mLua, -2, name.c_str());
	}
}


#endif //APP_LOCALCLASSPROXY_H
