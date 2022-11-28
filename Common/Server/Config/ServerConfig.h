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

    class ServerConfig : public Json::Reader, public TextConfig, public ConstSingleton<ServerConfig>
    {
    public:
        explicit ServerConfig();
    public:
        bool GetLocation(const char * name, std::string & location) const;
    protected:
        bool OnLoadText(const char *str, size_t length) final;
        bool OnReloadText(const char *str, size_t length) final;
	 public:
        const std::string& Name() const { return this->mName; }
		const std::string & GetContent() const { return this->mContent;}
		bool GetPath(const std::string & name, std::string & path) const;
        size_t GetServices(std::vector<std::string> & services, bool start = false) const;
    private:
        std::string mName;
		std::string mContent;
        std::unordered_map<std::string, std::string> mPaths;
        std::unordered_map<std::string , bool> mServiceConfigs;
    };
}