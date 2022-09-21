#pragma once

#include<vector>
#include<shared_mutex>
#include<unordered_map>
#include"XCode/XCode.h"
#include"rapidjson/document.h"
#include"InterfaceConfig.h"
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
		const T * GetConfig(const std::string & name) const;
		void GetConfigs(std::vector<const T *> & configs) const;
		virtual bool OnLoadConfig(const rapidjson::Value & json) = 0;
	 public:
		const std::string & GetName() const { return this->mName; }
	 protected:
		const std::string mName;
		std::unordered_map<std::string, T> mConfigs;
	};

	template<typename T>
	const T* IServiceConfig<T>::GetConfig(const std::string& name) const
	{
		auto iter = this->mConfigs.find(name);
		return iter != this->mConfigs.end() ? &iter->second : nullptr;
	}

	template<typename T>
	void IServiceConfig<T>::GetConfigs(std::vector<const T*>& configs) const
	{
		auto iter = this->mConfigs.begin();
		for(; iter != this->mConfigs.end(); iter++)
		{
			configs.emplace_back(&iter->second);
		}
	}

 	class RpcServiceConfig :  public IServiceConfig<RpcInterfaceConfig>
	{
	 public:
		using IServiceConfig::IServiceConfig;
	 public:
		bool OnLoadConfig(const rapidjson::Value &json) final;
	 public:
		static bool ParseFunName(const std::string & func, std::string & service, std::string & method);
	};

	class HttpServiceConfig : public IServiceConfig<HttpInterfaceConfig>
	{
	 public:
		using IServiceConfig::IServiceConfig;
	 public:
		bool OnLoadConfig(const rapidjson::Value &json) final;
	 private:
	};
}// namespace Sentry