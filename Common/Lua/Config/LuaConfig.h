//
// Created by leyi on 2023/6/9.
//

#ifndef APP_LUACONFIG_H
#define APP_LUACONFIG_H

#include"Yyjson/Object/JsonObject.h"
struct lua_State;

namespace lua
{
	struct Config : public json::Object<Config>
	{
	public:
		std::string main;
		std::vector<std::string> require;
		std::vector<std::string> modules;
	};
}


#endif //APP_LUACONFIG_H
