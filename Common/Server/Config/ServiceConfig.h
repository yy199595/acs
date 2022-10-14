#pragma once

#include<vector>
#include<shared_mutex>
#include<unordered_map>
#include"XCode/XCode.h"
#include"MethodConfig.h"
#include"Config/TextConfig.h"
#include"Singleton/Singleton.h"
#include"rapidjson/document.h"
namespace Sentry
{
	struct CodeConfig
	{
	 public:
		int Code;
		std::string Name;
		std::string Desc;
	};

    class IServiceConfigBase
    {
    public:
        IServiceConfigBase(const std::string & name) : mName(name) { }
        const std::string & GetName() const { return this->mName; }
    public:
        virtual bool IsRpcConfig() const = 0;
        virtual bool IsHttpConfig() const = 0;
        virtual const std::string & GetType() const = 0;
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
		using IServiceConfig::IServiceConfig;
    public:
        bool IsRpcConfig() const{ return true;}
        bool IsHttpConfig() const{ return false;}
        bool OnLoadConfig(const rapidjson::Value &json) final;
        bool OnReLoadConfig(const rapidjson::Value & json) final;
        const std::string & GetType() const { return this->mType; }
    private:
        std::string mType;
    };

	class HttpServiceConfig : public IServiceConfig<HttpMethodConfig>
	{
	 public:
		using IServiceConfig::IServiceConfig;
	 public:
        bool IsRpcConfig() const{ return false;}
        bool IsHttpConfig() const{ return true;}
        bool OnLoadConfig(const rapidjson::Value &json) final;
        bool OnReLoadConfig(const rapidjson::Value & json) final;
        const std::string & GetType() const { return this->mType; }
    private:
        std::string mType;
	};

    class ServiceConfig : public TextConfig, public ConstSingleton<ServiceConfig>
    {
    public:
        ServiceConfig() : TextConfig("ServiceConfig") { }
    public:
        bool OnLoadText(const std::string &content) final;
        bool OnReloadText(const std::string &content) final;
    public:
        const RpcServiceConfig * GetRpcConfig(const std::string & name) const;
        const HttpServiceConfig * GetHttpConfig(const std::string & name) const;
        size_t GetServiceConfigs(std::vector<const RpcServiceConfig *> & configs) const;
        size_t GetServiceConfigs(std::vector<const HttpServiceConfig *> & configs) const;
        const RpcMethodConfig * GetRpcMethodConfig(const std::string & fullName) const;
        const HttpMethodConfig * GetHttpMethodConfig(const std::string & path) const;
    private:
        std::unordered_map<std::string, const RpcMethodConfig *> mRpcMethodConfig;
        std::unordered_map<std::string, const HttpMethodConfig *> mHttpMethodConfig;
        std::unordered_map<std::string, std::unique_ptr<IServiceConfigBase>> mServiceConfigs;
    };

}// namespace Sentry