//
// Created by zmhy0073 on 2022/10/25.
//

#include"RedisConfig.h"
#include"Log/CommonLogDef.h"
#include"String/StringHelper.h"
#include"rapidjson/document.h"
namespace Sentry
{
    bool RedisConfig::OnLoadText(const char *str, size_t length)
	{
		rapidjson::Document document;
		if (document.Parse(str, length).HasParseError())
		{
			return false;
		}
		if (!document.HasMember("redis"))
		{
			return false;
		}
		const rapidjson::Value& jsonData = document["redis"];
		{
			this->mConfig.FreeClient = 30;
			this->mConfig.Index = jsonData["index"].GetInt();
			this->mConfig.Count = jsonData["count"].GetInt();
		}
        if(!jsonData.HasMember("address"))
        {
            return false;
        }
        for(unsigned int index = 0;index < jsonData["address"].Size();index++)
        {
            Net::Address addressInfo;
            std::string address(jsonData["address"].GetString());
            if(!Helper::String::ParseIpAddress(address, addressInfo.Ip, addressInfo.Port))
            {
                return false;
            }
            this->mConfig.Address.emplace_back(addressInfo);
        }
		if (jsonData.HasMember("free"))
		{
			this->mConfig.FreeClient = jsonData["free"].GetInt();
		}
		if (jsonData.HasMember("passwd"))
		{
			this->mConfig.Password = jsonData["passwd"].GetString();
		}
		if (jsonData.HasMember("scripts") && jsonData["scripts"].IsObject())
		{
			const rapidjson::Value& value = jsonData["scripts"];
			for (auto iter = value.MemberBegin(); iter != value.MemberEnd(); iter++)
			{
				const std::string key(iter->name.GetString());
				const std::string value(iter->value.GetString());
				this->mConfig.LuaFiles.emplace(key, this->WorkPath() + value);
			}
		}
		return true;
	}
}