#pragma once
#include<string>
#include<unordered_map>
#include"Config/Base/JsonConfig.h"
#include"Core/Singleton/Singleton.h"

namespace joke
{
	class ServerConfig : public JsonConfig, public ConstSingleton<ServerConfig>
    {
    public:
        explicit ServerConfig();
	private:
		bool OnLoadJson() final;
		bool OnReLoadJson() final;
		bool OnLoadText(const char *str, size_t length) final;
		bool OnReloadText(const char *str, size_t length) final;
	private:
		static void Append(json::w::Value & document, json::r::Value & doc);
	public:
		const std::string& Name() const { return this->mName; } //服务器名字
		bool GetPath(const std::string & name, std::string & path) const;
		const std::string & GetSecretKey() const { return this->mSecret; }
		std::unique_ptr<json::r::Document> Read(const std::string & name) const;
    private:
        std::string mName;
		std::string mSecret;
        std::unordered_map<std::string, std::string> mPaths;
        std::unordered_map<std::string, std::string> mLocations;
    };
}