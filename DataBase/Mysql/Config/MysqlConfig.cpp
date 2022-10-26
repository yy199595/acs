//
// Created by zmhy0073 on 2022/10/26.
//

#include"MysqlConfig.h"
#include"rapidjson/document.h"
namespace Sentry
{
    bool MysqlConfig::OnLoadText(const char *str, size_t length)
    {
        rapidjson::Document document;
        if(document.Parse(str, length).HasParseError())
        {
            return false;
        }
        this->Ip = document["ip"].GetString();
        this->Port = document["port"].GetInt();
        this->User = document["user"].GetString();
        this->MaxCount = document["count"].GetInt();
        this->Password = document["passwd"].GetString();
        return true;
    }

    bool MysqlConfig::OnReloadText(const char *str, size_t length)
    {
        return true;
    }
}