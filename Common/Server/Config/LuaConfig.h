//
// Created by leyi on 2023/6/9.
//

#ifndef APP_LUACONFIG_H
#define APP_LUACONFIG_H
#include<vector>
#include<string>
#include<rapidjson/document.h>
struct lua_State;
namespace Tendo
{
	class LuaConfig
	{
	public:
		bool Init(const rapidjson::Value & data);
		const std::string & Main() const { return this->mMain; }
		std::vector<std::string> Requires() const { return this->mRequires; }
		std::vector<std::string> LoadFiles() const { return this->mLoadfiles;}
	private:
		std::string mMain;
		std::vector<std::string> mRequires;
		std::vector<std::string> mLoadfiles;
	};
}


#endif //APP_LUACONFIG_H
