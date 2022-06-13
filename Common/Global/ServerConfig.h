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
		int FreeClient;
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
        explicit ServerConfig(int argc, char ** argv);
    public:
        bool LoadConfig();
        int GetNodeId() { return this->mNodeId; }
		const std::string& GetNodeName() { return this->mNodeName; }
		const ListenConfig * GetListen(const std::string & name) const;
		const RedisConfig * GetRedisConfig(const std::string & name) const;
		void GetRedisConfigs(std::vector<const RedisConfig *> & configs) const;
		void GetListeners(std::vector<const ListenConfig *> & listeners) const;
		bool GetListener(const std::string & name, std::string & address) const;
	 public:
		const std::string & GetContent() const { return this->mContent;}
		bool GetPath(const std::string & name, std::string & path) const;
		const std::string & GetExename() const { return this->mExePath;}
		const std::string & GetWorkPath() const { return this->mWrokDir; }
    private:
        int mNodeId;
		std::string mContent;
		std::string mExePath;
		std::string mWrokDir;
        std::string mNodeName;
		std::string mConfigPath;
		std::unordered_map<std::string, std::string> mPaths;
		std::unordered_map<std::string, ListenConfig *> mListens;
		std::unordered_map<std::string, RedisConfig> mRedisConfigs;
    };
}