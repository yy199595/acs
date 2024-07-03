//
// Created by leyi on 2023/6/9.
//

#include"LuaConfig.h"
namespace joke
{
	bool LuaConfig::Init(const json::r::Value& data)
	{
		data.Get("main", this->mMain);
		return data.Get("require", this->mRequires);
	}
}