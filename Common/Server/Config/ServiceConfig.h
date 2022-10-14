#pragma once

#include<vector>
#include<shared_mutex>
#include<unordered_map>
#include"XCode/XCode.h"
#include"rapidjson/document.h"
#include"MethodConfig.h"
#include"Config/TextConfig.h"
namespace Sentry
{
	struct CodeConfig
	{
	 public:
		int Code;
		std::string Name;
		std::string Desc;
	};

	template<typename T>
	class IServiceConfig
	{
	 public:
		IServiceConfig(const std::string & name) : mName(name) {}
	 public:
		const T * GetMethodConfig(const std::string & name) const;
		size_t GetMethodConfigs(std::vector<const T *> & configs) const;
		virtual bool OnLoadConfig(const rapidjson::Value & json) = 0;
	 public:
		const std::string & GetName() const { return this->mName; }
	 protected:
		const std::string mName;
		std::unordered_map<std::string, T> mMethodConfigs;
	};

	template<typename T>
	const T* IServiceConfig<T>::GetMethodConfig(const std::string& name) const
	{
		auto iter = this->mMethodConfigs.find(name);
		return iter != this->mMethodConfigs.end() ? &iter->second : nullptr;
	}

	template<typename T>
	size_t IServiceConfig<T>::GetMethodConfigs(std::vector<const T*>& configs) const
	{
		auto iter = this->mMethodConfigs.begin();
		for(; iter != this->mMethodConfigs.end(); iter++)
		{
            configs.emplace_back(&iter->second);
		}
        return configs.size();
	}

 	class RpcServiceConfig :  public IServiceConfig<RpcMethodConfig>
	{
	 public:
		using IServiceConfig::IServiceConfig;
	 public:
		bool OnLoadConfig(const rapidjson::Value &json) final;
    };

	class HttpServiceConfig : public IServiceConfig<HttpMethodConfig>
	{
	 public:
		using IServiceConfig::IServiceConfig;
	 public:
		bool OnLoadConfig(const rapidjson::Value &json) final;
	};

    class ServiceConfig : public TextConfig
    {
    public:
        ServiceConfig() : TextConfig("ServiceConfig") { }
    public:
        bool OnLoadText(const std::string &content) final { return true;}
        bool OnReloadText(const std::string &content) final { return true; }
    };

}// namespace Sentry