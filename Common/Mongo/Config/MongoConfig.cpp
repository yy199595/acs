//
// Created by zmhy0073 on 2022/10/26.
//

#include"MongoConfig.h"
#include"rapidjson/document.h"
#include"Util/String/StringHelper.h"
namespace Mongo
{
    bool MongoConfig::OnLoadText(const char *str, size_t length)
	{
		rapidjson::Document document;
		if (document.Parse(str, length).HasParseError())
		{
			return false;
		}
		if (!document.HasMember("mongo"))
		{
			return false;
		}
		const rapidjson::Value& jsonData = document["mongo"];
		{
			this->DB = jsonData["db"].GetString();
			this->User = jsonData["user"].GetString();
			this->MaxCount = jsonData["count"].GetInt();
			this->Password = jsonData["passwd"].GetString();
		}
		std::string net;
		Net::Address& addr = this->Address;
		addr.FullAddress.assign(jsonData["address"].GetString());
		if (!Helper::Str::SplitAddr(addr.FullAddress, net, addr.Ip, addr.Port))
		{
			return false;
		}
		return true;
	}

    bool MongoConfig::OnReloadText(const char *str, size_t length)
    {
        return true;
    }
}