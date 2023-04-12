#include "ServiceConfig.h"

#include"Entity/Unit/App.h"
#include"rapidjson/document.h"
#include"Cluster/Config/ClusterConfig.h"
namespace Tendo
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
				std::string server;
				const std::string & service = this->GetName();
                RpcMethodConfig * serviceConfig = this->mMethodConfigs[name].get();
                {
                    serviceConfig->Timeout = 0;
                    serviceConfig->Method = name;
                    serviceConfig->IsAsync = false;
                    serviceConfig->IsClient = false;
					serviceConfig->IsOpen = true;
                    serviceConfig->FullName = fullName;
                    serviceConfig->Service = this->GetName();
                }
				if(ClusterConfig::Inst()->GetServerName(service,server))
				{
					serviceConfig->Server = server;
				}
				if(jsonValue.HasMember("IsOpen"))
                {
                    serviceConfig->IsOpen = jsonValue["IsOpen"].GetBool();
                }
				if(jsonValue.HasMember("IsClient"))
				{
					serviceConfig->IsClient = jsonValue["IsClient"].GetBool();
				}
                if(jsonValue.HasMember("Async"))
                {
                    serviceConfig->IsAsync = jsonValue["Async"].GetBool();
                }
                if (jsonValue.HasMember("Request"))
                {
                    serviceConfig->Request = jsonValue["Request"].GetString();
                }
                if (jsonValue.HasMember("Response"))
                {
                    serviceConfig->Response = jsonValue["Response"].GetString();
                }
                if (jsonValue.HasMember("Timeout"))
                {
                    serviceConfig->Timeout = jsonValue["Timeout"].GetInt();
                }
				if(serviceConfig->IsClient)
				{
					this->mIsClient = true;
				}
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

namespace Tendo
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
                std::unique_ptr<HttpMethodConfig> serviceConfig(new HttpMethodConfig());
				serviceConfig->Method = name;
				serviceConfig->Service = this->GetName();
                if(jsonValue.HasMember("Type"))
                {
					serviceConfig->Type = jsonValue["Type"].GetString();
                }
				std::string server;
				serviceConfig->Path = jsonValue["Path"].GetString();
				serviceConfig->IsAsync = jsonValue["Async"].GetBool();
				if(ClusterConfig::Inst()->GetServerName(this->GetName(), server))
				{
					serviceConfig->Server = server;
				}
                if (jsonValue.HasMember("Content"))
                {
					serviceConfig->Content = jsonValue["Content"].GetString();
                }
                this->mMethodConfigs.emplace(serviceConfig->Method, std::move(serviceConfig));
            }
        }
		return true;
	}

    bool HttpServiceConfig::OnReLoadConfig(const rapidjson::Value &json)
    {
        return true;
    }
}

namespace Tendo
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

namespace Tendo
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
        return nullptr;
    }
}