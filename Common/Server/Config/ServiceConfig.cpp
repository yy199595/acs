#include "ServiceConfig.h"

#include "App/App.h"
#include "rapidjson/document.h"

namespace Sentry
{
	bool RpcServiceConfig::OnLoadConfig(const rapidjson::Value & json)
	{
		if(!json.IsObject())
		{
			return false;
		}
        this->mIsClient = false;
		this->mMethodConfigs.clear();
		auto iter = json.MemberBegin();
		for(; iter != json.MemberEnd(); iter++)
        {
            const rapidjson::Value &jsonValue = iter->value;
            if(jsonValue.IsObject())
            {
                const char *name = iter->name.GetString();
                std::string fullName = fmt::format("{0}.{1}", this->GetName(), name);
                if(this->mMethodConfigs.find(name) == this->mMethodConfigs.end())
                {
                    std::unique_ptr<RpcMethodConfig> config =
                            std::make_unique<RpcMethodConfig>();
                    this->mMethodConfigs.emplace(name, std::move(config));
                }
                RpcMethodConfig * serviceConfog = this->mMethodConfigs[name].get();
                {
                    serviceConfog->Method = name;
                    serviceConfog->IsAsync = false;
                    serviceConfog->Type = "Server";
                    serviceConfog->FullName = fullName;
                    serviceConfog->Service = this->GetName();
                }
                if(jsonValue.HasMember("Type"))
                {
                    serviceConfog->Type = jsonValue["Type"].GetString();
                }
                if(jsonValue.HasMember("Async"))
                {
                    serviceConfog->IsAsync = jsonValue["Async"].GetBool();
                }
                if (jsonValue.HasMember("Request"))
                {
                    serviceConfog->Request = jsonValue["Request"].GetString();
                }
                if (jsonValue.HasMember("Response"))
                {
                    serviceConfog->Response = jsonValue["Response"].GetString();
                }
                if(!this->mIsClient && serviceConfog->Type == "Client")
                {
                    this->mIsClient = true;
                }
                //this->mMethodConfigs.emplace(name, std::move(serviceConfog));
            }
        }
		return true;
	}

    bool RpcServiceConfig::OnReLoadConfig(const rapidjson::Value &json)
    {
        if(!json.IsObject())
        {
            return false;
        }
        return true;
    }
}

namespace Sentry
{
	bool HttpServiceConfig::OnLoadConfig(const rapidjson::Value & json)
	{
		if(!json.IsObject())
		{
			return false;
		}
        auto iter = json.MemberBegin();
		for(; iter != json.MemberEnd(); iter++)
        {
            const rapidjson::Value &jsonValue = iter->value;
            if(jsonValue.IsObject())
            {
                const char *name = iter->name.GetString();
                std::unique_ptr<HttpMethodConfig> serviceConfog(new HttpMethodConfig());
                serviceConfog->Method = name;
                serviceConfog->Service = this->GetName();
                if(jsonValue.HasMember("Type"))
                {
                    serviceConfog->Type = jsonValue["Type"].GetString();
                }
                serviceConfog->Path = jsonValue["Path"].GetString();
                serviceConfog->IsAsync = jsonValue["Async"].GetBool();
                if (jsonValue.HasMember("Content"))
                {
                    serviceConfog->Content = jsonValue["Content"].GetString();
                }
                this->mMethodConfigs.emplace(serviceConfog->Method, std::move(serviceConfog));
            }
        }
		return true;
	}

    bool HttpServiceConfig::OnReLoadConfig(const rapidjson::Value &json)
    {
        return true;
    }
}

namespace Sentry
{
    bool RpcConfig::OnLoadText(const char *str, size_t length)
    {
        rapidjson::Document document;
        if(document.Parse(str, length).HasParseError())
        {
            return false;
        }
        std::vector<const RpcMethodConfig *> methodConfigs;
        auto iter = document.MemberBegin();
        for(; iter != document.MemberEnd(); iter++)
        {
            const rapidjson::Value & jsonValue = iter->value;
            const std::string service(iter->name.GetString());
            if(this->mConfigs.find(service) == this->mConfigs.end())
            {
                std::unique_ptr<RpcServiceConfig> serviceConfig
                        = std::make_unique<RpcServiceConfig>(service);
                this->mConfigs.emplace(service, std::move(serviceConfig));
            }
            RpcServiceConfig * config = this->mConfigs[service].get();
            if(!config->OnLoadConfig(jsonValue))
            {
                LOG_ERROR("load rpc config " << service << " error");
                return false;
            }
            methodConfigs.clear();
            config->GetMethodConfigs(methodConfigs);
            for(const RpcMethodConfig * methodConfig : methodConfigs)
            {
                this->mRpcMethodConfig.emplace(methodConfig->FullName, methodConfig);
            }
        }
        return true;
    }

    bool RpcConfig::OnReloadText(const char *str, size_t length)
    {
        return this->OnLoadText(str, length);
    }

    const RpcServiceConfig *RpcConfig::GetConfig(const std::string &name) const
    {
        auto iter = this->mConfigs.find(name);
        return iter != this->mConfigs.end() ? iter->second.get() : nullptr;
    }

    const RpcMethodConfig *RpcConfig::GetMethodConfig(const std::string &fullName) const
    {
        auto iter = this->mRpcMethodConfig.find(fullName);
        return iter != this->mRpcMethodConfig.end() ? iter->second : nullptr;
    }
}

namespace Sentry
{
    bool HttpConfig::OnLoadText(const char *str, size_t length)
    {
        rapidjson::Document document;
        if(document.Parse(str, length).HasParseError())
        {
            return false;
        }
        std::vector<const HttpMethodConfig *> methodConfigs;
        auto iter = document.MemberBegin();
        for(; iter != document.MemberEnd(); iter++)
        {
            std::string service(iter->name.GetString());
            const rapidjson::Value & jsonValue = iter->value;
            std::unique_ptr<HttpServiceConfig> config(new HttpServiceConfig(service));
            if(!config->OnLoadConfig(jsonValue))
            {
                LOG_ERROR("load http config " << service << " error");
                return false;
            }
            methodConfigs.clear();
            config->GetMethodConfigs(methodConfigs);
            for(const HttpMethodConfig * methodConfig : methodConfigs)
            {
                this->mMethodConfigs.emplace(methodConfig->Path, methodConfig);
            }
            this->mConfigs.emplace(service, std::move(config));
        }
        return true;
    }

    bool HttpConfig::OnReloadText(const char *str, size_t length)
    {
        return true;
    }


    const HttpServiceConfig *HttpConfig::GetConfig(const std::string &name) const
    {
        auto iter = this->mConfigs.find(name);
        return iter != this->mConfigs.end() ? iter->second.get() : nullptr;
    }

    const HttpMethodConfig *HttpConfig::GetMethodConfig(const std::string & path) const
    {
        auto iter = this->mMethodConfigs.find(path);
        if(iter != this->mMethodConfigs.end())
        {
            return iter->second;
        }
        for(auto & value : this->mMethodConfigs)
        {
            if(path.find(value.second->Path) == 0)
            {
                return value.second;
            }
        }
        return nullptr;
    }
}