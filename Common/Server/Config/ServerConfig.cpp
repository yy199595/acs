#include<utility>
#ifdef __OS_WIN__
#include<direct.h>
#else
#include<unistd.h>
#endif // 

#include"ServerConfig.h"
#include"Log/CommonLogDef.h"
#include"File/FileHelper.h"
#include"spdlog/fmt/fmt.h"
#include"File/DirectoryHelper.h"
namespace Sentry
{
    ServerConfig::ServerConfig(const std::string & server)
        : TextConfig("ServerConfig"), mName(server)
    {

    }

    bool ServerConfig::OnReloadText(const char *str, size_t length)
    {
        return true;
    }

    bool ServerConfig::OnLoadText(const char *str, size_t length)
	{
		if (!this->ParseJson(str, length))
		{
            CONSOLE_LOG_ERROR("parse " << this->Path() << " failure");
            return false;
		}
        if (!this->HasMember(this->mName.c_str()))
        {
            return false;
        }
        
        auto iter = this->FindMember(this->mName.c_str());
        if(iter->value.HasMember("listen"))
        {
            const rapidjson::Value& document = iter->value["listen"];
            for (auto iter1 = document.MemberBegin(); iter1 != document.MemberEnd(); iter1++)
            {
                const std::string key(iter1->name.GetString());
                const std::string value(iter1->value.GetString());
                this->mListens.emplace(key, value);
            }
        }

        if (iter->value.HasMember("server"))
        {
            const rapidjson::Value& document = iter->value["server"];
            for (auto iter1 = document.MemberBegin(); iter1 != document.MemberEnd(); iter1++)
            {
                const std::string key(iter1->name.GetString());
                const std::string value(iter1->value.GetString());
                this->mLocations.emplace(key, value);
            }
        }

        if (iter->value.HasMember("path"))
        {
            const rapidjson::Value& document = iter->value["path"];
            for (auto iter1 = document.MemberBegin(); iter1 != document.MemberEnd(); iter1++)
            {
                const std::string key(iter1->name.GetString());
                const std::string value(iter1->value.GetString());
                this->mPaths.emplace(key, this->WorkPath() + value);
            }
        }

        if (this->HasMember("path") && (*this)["path"].IsObject())
		{
			const rapidjson::Value & jsonObject = (*this)["path"];
			auto iter1 = jsonObject.MemberBegin();
			for(; iter1 != jsonObject.MemberEnd(); iter1++)
			{
				const std::string key(iter1->name.GetString());
				const std::string value(iter1->value.GetString());
				this->mPaths.emplace(key, this->WorkPath() + value);
			}
		}
        return true;
	}

    bool ServerConfig::GetListen(const std::string& name, std::string& listen) const
    {
        auto iter = this->mListens.find(name);
        if (iter == this->mListens.end())
        {
            return false;
        }
        listen = iter->second;
        return true;
    }

    bool ServerConfig::GetLocation(const char *name, std::string &location) const
    {
        location.clear();   
        auto iter = this->mLocations.find(name);
        if (iter == this->mLocations.end())
        {
            return false;
        }
        location = iter->second;
        return true;
    }

	bool ServerConfig::GetPath(const std::string& name, std::string& path) const
	{
		auto iter = this->mPaths.find(name);
		if(iter != this->mPaths.end())
		{
			path = iter->second;
			return true;
		}
		return false;
	}
}
