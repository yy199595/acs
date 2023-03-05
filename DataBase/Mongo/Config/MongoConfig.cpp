//
// Created by zmhy0073 on 2022/10/26.
//

#include"MongoConfig.h"
#include"String/StringHelper.h"
#include"rapidjson/document.h"
namespace Sentry
{
    bool MongoConfig::OnLoadText(const char *str, size_t length) {
        rapidjson::Document document;
        if (document.Parse(str, length).HasParseError()) {
            return false;
        }
        if (!document.HasMember("mongo")) {
            return false;
        }
        const rapidjson::Value& jsonData = document["mongo"];
        {
            this->DB = jsonData["db"].GetString();
            this->User = jsonData["user"].GetString();
            this->MaxCount = jsonData["count"].GetInt();
            this->Password = jsonData["passwd"].GetString();
        }
        auto iter = jsonData.FindMember("address");
        if (iter == jsonData.MemberEnd() || !iter->value.IsArray())
        {
            for(unsigned int index = 0; iter->value.Size(); index++)
            {
                Net::Address addressInfo;
                std::string address(iter->value[index].GetString());
                Helper::String::ParseIpAddress(address, addressInfo.Ip, addressInfo.Port);
                this->Address.emplace_back(addressInfo);
            }
        }
        return true;
    }

    bool MongoConfig::OnReloadText(const char *str, size_t length)
    {
        return true;
    }
}