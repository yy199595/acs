#pragma once

#include<set>
#include<vector>
#include<string>
#include<unordered_map>
#include"Json/JsonReader.h"
#include"Config/TextConfig.h"
#include"Singleton/Singleton.h"
namespace Sentry
{
    struct ListenConfig
    {
    public:
        std::string Ip;
        std::string Name;
        std::string Address;
        unsigned short Port = 0;
    };

	struct ListenConfig;
    class ServerConfig : public Json::Reader, public TextConfig, public ConstSingleton<ServerConfig>
    {
    public:
        explicit ServerConfig();
    public:
        const ListenConfig * GetListenConfig(const char * name) const;
        bool GetLocation(const char * name, std::string & location) const;
    protected:
        bool OnLoadText(const std::string &content) final;
        bool OnReloadText(const std::string &content) final;
	 public:
        int GetNodeId() const { return this->mNodeId; }
        const std::string& GetNodeName() const { return this->mNodeName; }
		const std::string & GetContent() const { return this->mContent;}
		bool GetConfigPath(const std::string & name, std::string & path) const;
        size_t GetServices(std::vector<std::string> & services, bool start = false) const;
    private:
        int mNodeId;
		std::string mContent;
        std::string mNodeName;
        std::unordered_map<std::string, std::string> mPaths;
		std::unordered_map<std::string, ListenConfig> mListens;
        std::unordered_map<std::string , bool> mServiceConfigs;
    };
}