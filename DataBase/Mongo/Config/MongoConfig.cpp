//
// Created by zmhy0073 on 2022/10/26.
//

#include"MongoConfig.h"
#include"rapidjson/document.h"
namespace Sentry
{
    bool MongoConfig::OnLoadText(const char *str, size_t length)
    {
        rapidjson::Document document;
        if(document.Parse(str, length).HasParseError())
        {
            return false;
        }
        this->Ip = document["ip"].GetString();
        this->DB = document["db"].GetString();
        this->Port = document["port"].GetInt();
        this->User = document["user"].GetString();
        this->MaxCount = document["count"].GetInt();
        this->Password = document["passwd"].GetString();
        this->Address = this->Address + ":" + std::to_string(this->Port);
        return true;
    }

    bool MongoConfig::OnReloadText(const char *str, size_t length)
    {
        return true;
    }
}