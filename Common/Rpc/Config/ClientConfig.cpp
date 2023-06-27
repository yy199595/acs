//
// Created by leyi on 2023/6/16.
//

#include"ClientConfig.h"
#include"Log/Common/CommonLogDef.h"
namespace Tendo
{
	bool ClientConfig::OnLoadJson(rapidjson::Document &document)
	{
		auto iter = document.MemberBegin();
		for(; iter != document.MemberEnd(); iter++)
		{
			std::string name(iter->name.GetString());
			if(!iter->value.IsObject())
			{
				LOG_ERROR("parse json fail : " << name);
				return false;
			}
			auto iter1 = iter->value.MemberBegin();
			for(; iter1 != iter->value.MemberEnd(); iter1++)
			{
				std::string key(iter1->name.GetString());
				std::string value(iter1->value.GetString());
				this->mConfigs.emplace(fmt::format("{0}.{1}", name, key), value);
			}
		}
		return true;
	}

	bool ClientConfig::GetConfig(const std::string& name, std::string& response) const
	{
		auto iter = this->mConfigs.find(name);
		if(iter == this->mConfigs.end())
		{
			return false;
		}
		response = iter->second;
		return true;
	}

	bool ClientConfig::OnReLoadJson(rapidjson::Document &document)
	{
		return false;
	}
}