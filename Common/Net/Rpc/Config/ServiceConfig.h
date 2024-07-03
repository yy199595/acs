#pragma once

#include<vector>
#include<unordered_map>
#include"MethodConfig.h"
#include<unordered_set>
#include"Config/Base/JsonConfig.h"
#include"Core/Singleton/Singleton.h"
namespace joke
{
    class RpcConfig : public JsonConfig, public ConstSingleton<RpcConfig>
    {
    public:
        RpcConfig() : JsonConfig("RpcConfig") { }
	public:
		void GetRpcMethods(std::vector<std::string> & methods) const;
		const RpcMethodConfig * GetMethodConfig(const std::string & fullName) const;
		bool GetMethodConfigs(std::vector<const RpcMethodConfig *> & configs, int type) const;
		bool GetMethodConfigs(const std::string & name, std::vector<const RpcMethodConfig *> & configs) const;
		inline bool HasService(const std::string & name) const { return this->mAllServices.find(name) != this->mAllServices.end(); }
    private:
		bool OnLoadJson() final;
		bool OnReLoadJson() final;
    private:
		std::unordered_set<std::string> mAllServices;
        std::unordered_map<std::string, std::unique_ptr<RpcMethodConfig>> mRpcMethodConfig;
    };

    class HttpConfig : public JsonConfig , public Singleton<HttpConfig>
    {
    public:
        HttpConfig() : JsonConfig("HttpConfig") { }
    public:
		bool AddMethodConfig(std::unique_ptr<HttpMethodConfig> config);
        const HttpMethodConfig * GetMethodConfig(const std::string & url) const;
		void GetMethodList(std::vector<const HttpMethodConfig *> & configs, const std::string & url) const;
		void GetMethodConfigs(std::vector<const HttpMethodConfig *> & configs, const std::string & method) const;
		inline bool HasService(const std::string & name) const { return this->mAllService.find(name) != this->mAllService.end(); }
    private:
		bool OnLoadJson() final;
		bool OnReLoadJson() final;
    private:
		std::unordered_set<std::string> mAllService;
        std::unordered_map<std::string, std::unique_ptr<HttpMethodConfig>> mMethodConfigs;
    };
}// namespace Sentry