//
// Created by zmhy0073 on 2022/10/26.
//
#ifdef __ENABLE_MYSQL__


#include"MysqlConfig.h"
#include"Util/String/StringHelper.h"
#include"rapidjson/document.h"
namespace Tendo
{
    bool MysqlConfig::OnLoadText(const char *str, size_t length)
	{
		rapidjson::Document document;
		if (document.Parse(str, length).HasParseError())
		{
			return false;
		}
		if (!document.HasMember("mysql"))
		{
			return false;
		}
		rapidjson::Value& json = document["mysql"];
		this->User = json["user"].GetString();
		this->MaxCount = json["count"].GetInt();
		this->Password = json["passwd"].GetString();
		if (json.HasMember("ping"))
		{
			this->Ping = json["ping"].GetInt();
		}

		std::string net;
		Net::Address& addr = this->Address;
		addr.FullAddress.assign(json["address"].GetString());
		if (!Helper::Str::SplitAddr(addr.FullAddress, net, addr.Ip, addr.Port))
		{
			return false;
		}
		return true;
	}

    bool MysqlConfig::OnReloadText(const char *str, size_t length)
    {
        return true;
    }
}

#endif