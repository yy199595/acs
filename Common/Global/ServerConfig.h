#pragma once

#include<set>
#include<unordered_map>
#include"Json/JsonReader.h"
#include<Define/CommonTypeDef.h>

namespace Sentry
{
	struct RedisConfig
	{
	public:
		int Count;
		std::string Ip;
		std::string Name;
		unsigned short Port;
		std::string Address;
		std::string Password;
		std::vector<std::string> LuaFiles;
	};

	struct ListenConfig;
    class ServerConfig : public Json::Reader
    {
    public:
        explicit ServerConfig(std::string  path);
    public:
        bool LoadConfig();
        int GetNodeId() { return this->mNodeId; }
		const std::string& GetNodeName() { return this->mNodeName; }
		const ListenConfig * GetListen(const std::string & name) const;
		const RedisConfig * GetRedisConfig(const std::string & name) const;
		void GetRedisConfigs(std::vector<const RedisConfig *> & configs) const;
		void GetListeners(std::vector<const ListenConfig *> & listeners) const;
		bool GetListenerAddress(const std::string & name, std::string & address) const;
    private:   
        int mNodeId;
        std::string mNodeName;
        const std::string mConfigPath;
		std::unordered_map<std::string, ListenConfig *> mListens;
		std::unordered_map<std::string, RedisConfig> mRedisConfigs;
    };
}