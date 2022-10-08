#include<utility>
#include"ServerConfig.h"
#include"Log/CommonLogDef.h"
#include"File/FileHelper.h"
#include"spdlog/fmt/fmt.h"
#include"File/DirectoryHelper.h"
#include"Listener/TcpListenerComponent.h"
namespace Sentry
{
    ServerConfig::ServerConfig(int argc, char ** argv)
        : mExePath(argv[0]), mWrokDir(getcwd(NULL, 0))
    {
        this->mNodeId = 0;
        this->mWrokDir += "/";
        this->mConfigPath = argv[1];
        std::cout << "Exe Path : " << this->mExePath << std::endl;
        std::cout << "Work Path : " << this->mWrokDir << std::endl;
    }

    bool ServerConfig::LoadConfig()
	{
		if (!Helper::File::ReadTxtFile(this->mConfigPath, this->mContent))
		{
            CONSOLE_LOG_ERROR("not find config " << this->mConfigPath);
            return false;
		}
		if (!this->ParseJson(this->mContent))
		{
            CONSOLE_LOG_ERROR("parse " << this->mConfigPath << " failure");
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
            this->GetListener("rpc", this->mLocalHost);
        }
        if(this->GetJsonValue("services") != nullptr)
        {
            std::unordered_map<std::string, const rapidjson::Value *> services;
            IF_THROW_ERROR(this->GetMember("services", services));
            for (auto iter = services.begin(); iter != services.end(); iter++)
            {
                const std::string & name = iter->first;
                const rapidjson::Value &jsonObject = *iter->second;
                if(jsonObject.IsObject())
                {
                    ServiceConfig serviceConfig;
                    if(!jsonObject.HasMember("Type"))
                    {
                        CONSOLE_LOG_FATAL(name << " not config [Type]");
                        return false;
                    }
                    if(!jsonObject.HasMember("IsStart"))
                    {
                        CONSOLE_LOG_FATAL(name << " not config [IsStart]");
                        return false;
                    }
                    serviceConfig.Type = jsonObject["Type"].GetString();
                    serviceConfig.IsStart = jsonObject["IsStart"].GetBool();
                    if(jsonObject.HasMember("Address"))
                    {
                        serviceConfig.Address = jsonObject["Address"].GetString();
                    }
                    serviceConfig.Name = name;
                    this->mServiceConfigs.emplace(name, serviceConfig);
                }
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
				this->mPaths.emplace(key, this->mWrokDir + value);
			}
		}
		return true;
	}

    const ServiceConfig *ServerConfig::GetServiceConfig(const std::string &name) const
    {
        auto iter = this->mServiceConfigs.find(name);
        return iter != this->mServiceConfigs.end() ? &iter->second : nullptr;
    }

    size_t ServerConfig::GetServiceConfigs(std::vector<const ServiceConfig *> &configs) const
    {
        auto iter = this->mServiceConfigs.begin();
        for(; iter != this->mServiceConfigs.end(); iter++)
        {
            configs.emplace_back(&iter->second);
        }
        return configs.size();
    }

    const ListenConfig *ServerConfig::GetListenConfig(const char *name) const
    {
        auto iter = this->mListens.find(name);
        return iter != this->mListens.end() ? &iter->second : nullptr;
    }

	bool ServerConfig::GetListener(const std::string& name, std::string& address) const
	{
		auto iter = this->mListens.find(name);
		if(iter != this->mListens.end())
		{
			address = iter->second.Address;
			return true;
		}
		return false;
	}

	bool ServerConfig::GetPath(const string& name, string& path) const
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
