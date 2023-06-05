//
// Created by zmhy0073 on 2022/10/25.
//

#include"RedisConfig.h"
#include"rapidjson/document.h"
#include"Util/String/StringHelper.h"
namespace Tendo
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

		std::string net;
		Net::Address& addr = this->mConfig.Address;
		addr.FullAddress.assign(jsonData["address"].GetString());
		if (!Helper::Str::SplitAddr(addr.FullAddress, net, addr.Ip, addr.Port))
		{
			return false;
		}
		
		if (jsonData.HasMember("free"))
		{
			this->mConfig.FreeClient = jsonData["free"].GetInt();
		}
		if (jsonData.HasMember("passwd"))
		{
			this->mConfig.Password = jsonData["passwd"].GetString();
		}
		if (jsonData.HasMember("script"))
		{
			this->mConfig.Script = jsonData["script"].GetString();
			this->mConfig.Script = this->WorkPath() + this->mConfig.Script;
		}
		return true;
	}
}