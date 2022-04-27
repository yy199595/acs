#include<utility>
#include"ServerConfig.h"
#include<Define/CommonLogDef.h>
#include<Util/FileHelper.h>
#include<spdlog/fmt/fmt.h>
#include"Util/DirectoryHelper.h"
#include"Util/StringHelper.h"
#include"Network/Listener/NetworkListener.h"
namespace Sentry
{
    ServerConfig::ServerConfig(std::string  path)
            : mConfigPath(std::move(path))
    {
        this->mNodeId = 0;
    }

    bool ServerConfig::LoadConfig()
	{
		std::string outString;
		if (!Helper::File::ReadTxtFile(this->mConfigPath, outString))
		{
			throw std::logic_error("not find config : " + mConfigPath);
			return false;
		}
		if (!this->ParseJson(outString))
		{
			throw std::logic_error("parse json : " + mConfigPath + " failure");
			return false;
		}

		IF_THROW_ERROR(this->GetJsonValue("listener", "rpc"));
		//IF_THROW_ERROR(this->GetJsonValue("listener", "http"));
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
				IF_THROW_ERROR(jsonObject.HasMember("count"));
				IF_THROW_ERROR(jsonObject.HasMember("component"));

				listenConfig->Name = iter->first;
				listenConfig->Ip = jsonObject["ip"].GetString();
				listenConfig->Port = jsonObject["port"].GetUint();
				listenConfig->Count = jsonObject["count"].GetInt();
				listenConfig->Token = Helper::String::CreateNewToken();
				listenConfig->Handler = jsonObject["component"].GetString();;
				listenConfig->Address = fmt::format("{0}:{1}", listenConfig->Ip, listenConfig->Port);
				this->mListens.emplace(listenConfig->Name, listenConfig);
			}
		}

		std::unordered_map<std::string, const rapidjson::Value*> redisConfigs;
		IF_THROW_ERROR(this->GetMember("redis", redisConfigs));
		for (auto iter = redisConfigs.begin(); iter != redisConfigs.end(); iter++)
		{
			const rapidjson::Value& jsonObject = *iter->second;
			IF_THROW_ERROR(jsonObject.IsObject());

			RedisConfig redisConfig;
			redisConfig.Count = 3;
			redisConfig.Name = iter->first;
			redisConfig.Ip = jsonObject["ip"].GetString();
			redisConfig.Port = jsonObject["port"].GetInt();
			if (jsonObject.HasMember("count"))
			{
				redisConfig.Count = jsonObject["count"].GetInt();
			}
			if (jsonObject.HasMember("passwd"))
			{
				redisConfig.Password = jsonObject["passwd"].GetString();
			}
			if (jsonObject.HasMember("lua"))
			{
				for (int index = 0; index < jsonObject["lua"].Size(); index++)
				{
					std::string lua = jsonObject["lua"][index].GetString();
					redisConfig.LuaFiles.emplace_back(lua);
				}
			}
			redisConfig.Address = fmt::format("{0}:{1}", redisConfig.Ip, redisConfig.Port);
			this->mRedisConfigs.emplace(iter->first, redisConfig);
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

	bool ServerConfig::GetListenerAddress(const std::string& name, std::string& address) const
	{
		auto iter = this->mListens.find(name);
		if(iter != this->mListens.end())
		{
			address = iter->second->Address;
			return true;
		}
		return false;
	}

	const ListenConfig* ServerConfig::GetListen(const std::string& name) const
	{
		auto iter = this->mListens.find(name);
		return iter != this->mListens.end() ? iter->second : nullptr;
	}

	const RedisConfig* ServerConfig::GetRedisConfig(const string& name) const
	{
		auto iter = this->mRedisConfigs.find(name);
		return iter != this->mRedisConfigs.end() ? &iter->second : nullptr;
	}

	void ServerConfig::GetRedisConfigs(std::vector<const RedisConfig*>& configs) const
	{
		auto iter = this->mRedisConfigs.begin();
		for(; iter != this->mRedisConfigs.end(); iter++)
		{
			configs.emplace_back(&iter->second);
		}
	}
}
