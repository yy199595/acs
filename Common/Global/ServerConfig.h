#pragma once

#include<set>
#include<vector>
#include<unordered_map>
#include"Json/JsonReader.h"
#include<Define/CommonTypeDef.h>

namespace Sentry
{
	struct RedisConfig
	{
	public:
		int Count;
        int Index;
		int FreeClient;
		std::string Ip;
		std::string Name;
		unsigned short Port;
		std::string Address;
		std::string Password;
        std::vector<std::string> Channels;
        std::vector<std::string> LuaFiles;
	};

    struct ServiceConfig
    {
    public:
        bool IsStart;
        std::string Type;
        std::string Name;
        std::string Address;
    };

	struct ListenConfig;
    class ServerConfig : public Json::Reader
    {
    public:
        explicit ServerConfig(int argc, char ** argv);
    public:
        bool LoadConfig();
		void GetListeners(std::vector<const ListenConfig *> & listeners) const;
		bool GetListener(const std::string & name, std::string & address) const;
	 public:
        int GetNodeId() const { return this->mNodeId; }
        const std::string& GetNodeName() const { return this->mNodeName; }
		const std::string & GetContent() const { return this->mContent;}
		bool GetPath(const std::string & name, std::string & path) const;
		const std::string & GetExename() const { return this->mExePath;}
		const std::string & GetWorkPath() const { return this->mWrokDir; }
        const ServiceConfig * GetServiceConfig(const std::string & name) const;
        size_t GetServiceConfigs(std::vector<const ServiceConfig *> & configs) const;
    private:
        int mNodeId;
		std::string mContent;
		std::string mExePath;
		std::string mWrokDir;
        std::string mNodeName;
		std::string mConfigPath;
		std::unordered_map<std::string, std::string> mPaths;
		std::unordered_map<std::string, ListenConfig> mListens;
        std::unordered_map<std::string , ServiceConfig> mServiceConfigs;
    };
}