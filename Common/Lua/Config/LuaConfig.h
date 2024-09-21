//
// Created by leyi on 2023/6/9.
//

#ifndef APP_LUACONFIG_H
#define APP_LUACONFIG_H
#include<vector>
#include<string>
#include"Yyjson/Document/Document.h"

struct lua_State;
namespace acs
{
	class LuaConfig
	{
	public:
		bool Init(const json::r::Value & data);
		const std::string & Main() const { return this->mMain; }
		std::vector<std::string> Requires() const { return this->mRequires; }
	private:
		std::string mMain;
		std::vector<std::string> mRequires;
	};
}


#endif //APP_LUACONFIG_H
