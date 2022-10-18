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
                if(!this->mIsClient && serviceConfog->Type == "Client")
                {
                    this->mIsClient = true;
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
    bool RpcConfig::OnLoadText(const std::string &content)
    {
        rapidjson::Document document;
        const char * str = content.c_str();
        const size_t length = content.size();
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
            std::unique_ptr<RpcServiceConfig> config(new RpcServiceConfig(service));
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
            this->mConfigs.emplace(service, std::move(config));
        }
        return true;
    }

    bool RpcConfig::OnReloadText(const std::string &content)
    {
        return true;
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
    bool HttpConfig::OnLoadText(const std::string &content)
    {
        rapidjson::Document document;
        const char * str = content.c_str();
        const size_t length = content.size();
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

    bool HttpConfig::OnReloadText(const std::string &content)
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