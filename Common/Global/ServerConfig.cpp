#include<utility>
#include"ServerConfig.h"
#include<Define/CommonLogDef.h>
#include<Util/FileHelper.h>
#include<spdlog/fmt/fmt.h>
#include"Util/DirectoryHelper.h"
#include"Network/Listener/TcpServerListener.h"
namespace Sentry
{
    ServerConfig::ServerConfig(int argc, char ** argv)
        : mExePath(argv[0]), mWrokDir(getcwd(NULL, NULL))
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
			throw std::logic_error("not find config : " + mConfigPath);
		}
		if (!this->ParseJson(this->mContent))
		{
			throw std::logic_error("parse json : " + mConfigPath + " failure");
		}

		IF_THROW_ERROR(this->GetJsonValue("listener", "rpc"));
		IF_THROW_ERROR(this->GetMember("area_id", this->mNodeId));
		IF_THROW_ERROR(this->GetMember("node_name", this->mNodeName));

		std::unordered_map<std::string, const rapidjson::Value*> listeners;
		IF_THROW_ERROR(this->GetMember("listener", listeners));
		for (auto iter = listeners.begin(); iter != listeners.end(); iter++)
		{
			const rapidjson::Value& jsonObject = *iter->second;
			if (jsonObject.IsObject())
			{
				ListenConfig* listenConfig = new ListenConfig();
				IF_THROW_ERROR(jsonObject.HasMember("ip"));
				IF_THROW_ERROR(jsonObject.HasMember("port"));
				IF_THROW_ERROR(jsonObject.HasMember("component"));

				listenConfig->Name = iter->first;
				listenConfig->Ip = jsonObject["ip"].GetString();
				listenConfig->Port = jsonObject["port"].GetUint();
				listenConfig->Handler = jsonObject["component"].GetString();;
				listenConfig->Address = fmt::format("{0}:{1}", listenConfig->Ip, listenConfig->Port);
				this->mListens.emplace(listenConfig->Name, listenConfig);
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

	void ServerConfig::GetListeners(std::vector<const ListenConfig*>& listeners) const
	{
		listeners.clear();
		auto iter = this->mListens.begin();
		for (; iter != this->mListens.end(); iter++)
		{
			listeners.emplace_back(iter->second);
		}
	}

	bool ServerConfig::GetListener(const std::string& name, std::string& address) const
	{
		auto iter = this->mListens.find(name);
		if(iter != this->mListens.end())
		{
			address = iter->second->Address;
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
