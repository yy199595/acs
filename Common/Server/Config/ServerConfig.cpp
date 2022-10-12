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
				this->mPaths.emplace(key, this->mWrokDir + value);
			}
		}
		return true;
	}

    size_t ServerConfig::GetServices(std::vector<std::string> &services, bool start) const
    {
        for(auto & value : this->mServiceConfigs)
        {
            if(start && value.second)
            {
                services.emplace_back(value.first);
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
