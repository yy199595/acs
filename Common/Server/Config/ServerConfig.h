#pragma once

#include<set>
#include<vector>
#include<string>
#include<unordered_map>
#include"Util/Json/JsonReader.h"
#include"Server/Config/TextConfig.h"
#include"Core/Singleton/Singleton.h"
namespace Tendo
{
    struct ListenConfig
    {
    public:
        std::string Ip;
        std::string Net;
        unsigned short Port = 0;
    };


    class ServerConfig : public Json::Reader, public TextConfig, public ConstSingleton<ServerConfig>
    {
    public:
        explicit ServerConfig();
    public:
		bool GetListen(std::vector<std::string> & names) const;
		bool GetListen(const std::string & name, unsigned short & port) const;
		bool GetListen(const std::string & name, std::string & net, unsigned short & port) const;
		bool GetLocation(const char * name, std::string & location) const;
    protected:
        bool OnLoadText(const char *str, size_t length) final;
        bool OnReloadText(const char *str, size_t length) final;
	 public:
		int GroupId() const { return this->mGroupId; }
		int ServerId() const { return this->mServerId;}
		const std::string& Name() const { return this->mName; } //服务器名字
		const std::string & GetContent() const { return this->mContent;}
		bool GetPath(const std::string & name, std::string & path) const;
    private:
		int mGroupId;
		int mServerId;
        std::string mName;
		std::string mContent;
        std::unordered_map<std::string, ListenConfig> mListens;
        std::unordered_map<std::string, std::string> mPaths;
        std::unordered_map<std::string, std::string> mLocations;
    };
}