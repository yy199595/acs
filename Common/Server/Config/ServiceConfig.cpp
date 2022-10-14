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
		this->mMethodConfigs.clear();
        this->mType = json["Type"].GetString();
		auto iter = json.MemberBegin();
		for(; iter != json.MemberEnd(); iter++)
        {
            const rapidjson::Value &jsonValue = iter->value;
            if(jsonValue.IsObject())
            {
                const char *name = iter->name.GetString();
                std::unique_ptr<RpcMethodConfig> serviceConfog(new RpcMethodConfig());

                serviceConfog->Method = name;
                serviceConfog->Service = this->GetName();
                serviceConfog->Type = jsonValue["Type"].GetString();
                serviceConfog->IsAsync = jsonValue["Async"].GetBool();
                if (jsonValue.HasMember("Request"))
                {
                    serviceConfog->Request = jsonValue["Request"].GetString();
                }
                if (jsonValue.HasMember("Response"))
                {
                    serviceConfog->Response = jsonValue["Response"].GetString();
                }
                serviceConfog->FullName = fmt::format("{0}.{1}", this->GetName(), name);
                this->mMethodConfigs.emplace(name, std::move(serviceConfog));
            }
        }
		return true;
	}

    bool RpcServiceConfig::OnReLoadConfig(const rapidjson::Value &json)
    {
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
        this->mType = json["Type"].GetString();
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
                serviceConfog->Type = jsonValue["Type"].GetString();
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
    bool ServiceConfig::OnLoadText(const std::string &content)
    {
        rapidjson::Document document;
        const char * str = content.c_str();
        const size_t length = content.size();
        if(document.Parse(str, length).HasParseError())
        {
            return false;
        }
        auto iter = document.MemberBegin();
        for(; iter != document.MemberEnd(); iter++)
        {
            const rapidjson::Value & jsonValue = iter->value;
            const std::string service(iter->name.GetString());
            if(!jsonValue.HasMember("Type"))
            {
                LOG_ERROR(service << " not find 'Type' field");
                return false;
            }
            if(this->mServiceConfigs.find(service) != this->mServiceConfigs.end())
            {
                LOG_ERROR(service << " config existing");
                return false;
            }

            const std::string type(jsonValue["Type"].GetString());
            if(type == "rpc")
            {
                std::unique_ptr<RpcServiceConfig> config(new RpcServiceConfig(service));
                if(!config->OnLoadConfig(jsonValue))
                {
                    LOG_ERROR("load rpc config " << service << " error");
                    return false;
                }
                this->mServiceConfigs.emplace(service, std::move(config));
            }
            else if(type == "http")
            {
                std::unique_ptr<HttpServiceConfig> config(new HttpServiceConfig(service));
                if(!config->OnLoadConfig(jsonValue))
                {
                    LOG_ERROR("load http config " << service << " error");
                    return false;
                }
                this->mServiceConfigs.emplace(service, std::move(config));
            }
            else
            {
                LOG_ERROR(service << " unknow type : " << type);
                return false;
            }
        }

        for(auto & value : this->mServiceConfigs)
        {
            if(value.second->IsRpcConfig())
            {
                std::vector<const RpcMethodConfig *> methodConfigs;
                const RpcServiceConfig *config = value.second->Cast<RpcServiceConfig>();
                if (config->GetMethodConfigs(methodConfigs) > 0)
                {
                    for(const RpcMethodConfig * methodConfig : methodConfigs)
                    {
                        this->mRpcMethodConfig.emplace(methodConfig->FullName, methodConfig);
                    }
                }
            }
            else if(value.second->IsHttpConfig())
            {
                std::vector<const HttpMethodConfig *> methodConfigs;
                const HttpServiceConfig * config = value.second->Cast<HttpServiceConfig>();
                if(config->GetMethodConfigs(methodConfigs) > 0)
                {
                    for(const HttpMethodConfig * methodConfig : methodConfigs)
                    {
                        this->mHttpMethodConfig.emplace(methodConfig->Path, methodConfig);
                    }
                }
            }
        }
        return true;
    }

    bool ServiceConfig::OnReloadText(const std::string &content)
    {
        return true;
    }

    const RpcServiceConfig *ServiceConfig::GetRpcConfig(const std::string &name) const
    {
        auto iter = this->mServiceConfigs.find(name);
        if(iter != this->mServiceConfigs.end() && iter->second->IsRpcConfig())
        {
            return iter->second->Cast<RpcServiceConfig>();
        }
        return nullptr;
    }

    const HttpServiceConfig *ServiceConfig::GetHttpConfig(const std::string &name) const
    {
        auto iter = this->mServiceConfigs.find(name);
        if(iter != this->mServiceConfigs.end() && iter->second->IsHttpConfig())
        {
            return iter->second->Cast<HttpServiceConfig>();
        }
        return nullptr;
    }

    size_t ServiceConfig::GetServiceConfigs(std::vector<const RpcServiceConfig *> &configs) const
    {
        for(auto & value : this->mServiceConfigs)
        {
            if(value.second->IsRpcConfig())
            {
                configs.emplace_back(value.second->Cast<RpcServiceConfig>());
            }
        }
        return configs.size();
    }

    size_t ServiceConfig::GetServiceConfigs(std::vector<const HttpServiceConfig *> &configs) const
    {
        for(auto & value : this->mServiceConfigs)
        {
            if(value.second->IsRpcConfig())
            {
                configs.emplace_back(value.second->Cast<HttpServiceConfig>());
            }
        }
        return configs.size();
    }

    const RpcMethodConfig *ServiceConfig::GetRpcMethodConfig(const std::string &fullName) const
    {
        auto iter = this->mRpcMethodConfig.find(fullName);
        return iter != this->mRpcMethodConfig.end() ? iter->second : nullptr;
    }

    const HttpMethodConfig *ServiceConfig::GetHttpMethodConfig(const std::string &path) const
    {
        auto iter = this->mHttpMethodConfig.find(path);
        if(iter != this->mHttpMethodConfig.end())
        {
            return iter->second;
        }
        for(auto & value : this->mHttpMethodConfig)
        {
            if(path.find(value.second->Path) == 0)
            {
                return value.second;
            }
        }
        return nullptr;
    }
}
