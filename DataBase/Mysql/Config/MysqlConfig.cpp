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
        if (document.Parse(str, length).HasParseError())
        {
            return false;
        }
		if(!document.HasMember("mysql"))
		{
			return false;
		}
        this->Ip = document["mysql"]["ip"].GetString();
        this->Port = document["mysql"]["port"].GetInt();
        this->User = document["mysql"]["user"].GetString();
        this->MaxCount = document["mysql"]["count"].GetInt();
        this->Password = document["mysql"]["passwd"].GetString();
        this->Address = this->Ip + ":" + std::to_string(this->Port);
        return true;
    }

    bool MysqlConfig::OnReloadText(const char *str, size_t length)
    {
        return true;
    }
}