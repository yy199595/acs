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
    ServerConfig::ServerConfig()
        : TextConfig("ServerConfig")
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
        const rapidjson::Value  * value1 = this->GetJsonValue("services");
        if(value1 != nullptr && value1->IsObject())
        {
            auto iter = value1->MemberBegin();
            for(; iter != value1->MemberEnd(); iter++)
            {
                bool start = iter->value.GetBool();
                const std::string name(iter->name.GetString());
                this->mServiceConfigs.emplace(name, start);
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

    bool ServerConfig::GetLocation(const char *name, std::string &location) const
    {
        location.clear();
        if(!this->GetMember("server", name, location))
        {
            CONSOLE_LOG_FATAL("not find location : " << name);
            return false;
        }
        return true;
    }

    size_t ServerConfig::GetServices(std::vector<std::string> &services, bool start) const
    {
        for(auto & value : this->mServiceConfigs)
        {
            if(start && !value.second)
            {
                continue;
            }
            services.emplace_back(value.first);
        }
        return services.size();
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
