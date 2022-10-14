#include<utility>
#include <unistd.h>
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
        this->mNodeId = 0;
    }

    bool ServerConfig::OnReloadText(const std::string &content)
    {
        return true;
    }

    bool ServerConfig::OnLoadText(const std::string &content)
	{
		if (!this->ParseJson(content))
		{
            CONSOLE_LOG_ERROR("parse " << this->GetPath() << " failure");
            return false;
		}
        this->GetMember("area_id",this->mNodeId);
        this->GetMember("node_name",this->mNodeName);

        if(this->GetJsonValue("listener") != nullptr)
        {
            IF_THROW_ERROR(this->GetJsonValue("listener","rpc"));
            const rapidjson::Value * json = this->GetJsonValue("listener");
            for (auto iter = json->MemberBegin(); iter != json->MemberEnd(); iter++)
            {
                const rapidjson::Value& jsonObject = iter->value;
                if (jsonObject.IsObject())
                {
                    ListenConfig listenConfig;
                    listenConfig.Name = iter->name.GetString();
                    listenConfig.Ip = jsonObject["ip"].GetString();
                    listenConfig.Port = jsonObject["port"].GetUint();                  
                    if(jsonObject.HasMember("route"))
                    {
                        listenConfig.Route = jsonObject["route"].GetString();
                    }
                    listenConfig.Address = fmt::format("{0}:{1}",listenConfig.Ip,listenConfig.Port);
                    this->mListens.emplace(listenConfig.Name, listenConfig);
                }
            }
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

        const std::string workDir(getcwd(NULL, NULL));
        if (this->HasMember("path") && (*this)["path"].IsObject())
		{
			const rapidjson::Value & jsonObject = (*this)["path"];
			auto iter1 = jsonObject.MemberBegin();
			for(; iter1 != jsonObject.MemberEnd(); iter1++)
			{
				const std::string key(iter1->name.GetString());
				const std::string value(iter1->value.GetString());
				this->mPaths.emplace(key, fmt::format("{0}/{1}", workDir, value));
			}
		}
		return true;
	}

    bool ServerConfig::GetLocation(const char *name, std::string &location) const
    {
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

    const ListenConfig *ServerConfig::GetListenConfig(const char *name) const
    {
        auto iter = this->mListens.find(name);
        return iter != this->mListens.end() ? &iter->second : nullptr;
    }

	bool ServerConfig::GetConfigPath(const std::string& name, std::string& path) const
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
