#pragma once

#include <utility>
#include<vector>
#include<shared_mutex>
#include<unordered_map>
#include"MethodConfig.h"
#include"Config/Base/JsonConfig.h"
#include"Core/Singleton/Singleton.h"
namespace Tendo
{
    class IServiceConfigBase
    {
    public:
        explicit IServiceConfigBase(std::string  name) : mName(std::move(name)) { }
        const std::string & GetName() const { return this->mName; }
    public:
        virtual bool OnLoadConfig(const rapidjson::Value & json) = 0;
        virtual bool OnReLoadConfig(const rapidjson::Value & json) = 0;

        template<typename T>
        const T * Cast() { return static_cast<T*>(this); }
    private:
        std::string mName;
        std::string nType;
    };

	template<typename T>
    class IServiceConfig : public IServiceConfigBase
	{
	 public:
		IServiceConfig(const std::string & name) : IServiceConfigBase(name) {}
	 public:
		const T * GetMethodConfig(const std::string & name) const;
		size_t GetMethodConfigs(std::vector<const T *> & configs) const;
    protected:
		std::unordered_map<std::string, std::unique_ptr<T>> mMethodConfigs;
	};

	template<typename T>
	const T* IServiceConfig<T>::GetMethodConfig(const std::string& name) const
	{
		auto iter = this->mMethodConfigs.find(name);
		return iter != this->mMethodConfigs.end() ? iter->second.get() : nullptr;
	}

	template<typename T>
	size_t IServiceConfig<T>::GetMethodConfigs(std::vector<const T*>& configs) const
	{
		auto iter = this->mMethodConfigs.begin();
		for(; iter != this->mMethodConfigs.end(); iter++)
		{
            configs.emplace_back(iter->second.get());
		}
        return configs.size();
	}

 	class RpcServiceConfig :  public IServiceConfig<RpcMethodConfig>
	{
	 public:
		RpcServiceConfig(const std::string & name);
    public:
        bool IsClient() const { return this->mIsClient; }
        bool OnLoadConfig(const rapidjson::Value &json) final;
        bool OnReLoadConfig(const rapidjson::Value & json) final;
    private:
        bool mIsClient;
		std::unordered_map<std::string, int> mNets;
    };

	class HttpServiceConfig : public IServiceConfig<HttpMethodConfig>
	{
	 public:
		using IServiceConfig::IServiceConfig;
	 public:
        bool OnLoadConfig(const rapidjson::Value &json) final;
        bool OnReLoadConfig(const rapidjson::Value & json) final;
	};

    class SrvRpcConfig : public JsonConfig, public ConstSingleton<SrvRpcConfig>
    {
    public:
        SrvRpcConfig() : JsonConfig("RpcConfig") { }

    public:
        const RpcServiceConfig * GetConfig(const std::string & name) const;
        const RpcMethodConfig * GetMethodConfig(const std::string & fullName) const;
    private:
		bool OnLoadJson(rapidjson::Document &document) final;
		bool OnReLoadJson(rapidjson::Document &document) final;
    private:
        std::unordered_map<std::string, const RpcMethodConfig *> mRpcMethodConfig;
        std::unordered_map<std::string, std::unique_ptr<RpcServiceConfig>> mConfigs;
    };

    class HttpConfig : public JsonConfig , public ConstSingleton<HttpConfig>
    {
    public:
        HttpConfig() : JsonConfig("HttpConfig") { }
    public:
        const HttpServiceConfig * GetConfig(const std::string & name) const;
        const HttpMethodConfig * GetMethodConfig(const std::string & fullName) const;
    private:
		bool OnLoadJson(rapidjson::Document &document) final;
		bool OnReLoadJson(rapidjson::Document &document) final;
    private:
        std::unordered_map<std::string, const HttpMethodConfig *> mMethodConfigs;
        std::unordered_map<std::string, std::unique_ptr<HttpServiceConfig>> mConfigs;
    };
}// namespace Sentry