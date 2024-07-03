#include "ServiceConfig.h"
#include"Rpc/Client/Message.h"
#include"Util/String/String.h"
#include"Cluster/Config/ClusterConfig.h"

namespace joke
{
    bool RpcConfig::OnLoadJson()
    {
		std::vector<const char *> keys;
		if(this->GetKeys(keys) <= 0)
		{
			return false;
		}
		std::unique_ptr<json::r::Value> value;
		for(const char * key : keys)
		{
			if(!this->Get(key, value))
			{
				return false;
			}
			std::string service, func;
			const std::string method(key);
			std::unique_ptr<RpcMethodConfig> methodConfig = std::make_unique<RpcMethodConfig>();
			{
				if (help::Str::Split(method, '.', service, func) != 0)
				{
					LOG_ERROR("rpc config : {}", method);
					return false;
				}
				methodConfig->Timeout = 0;
				methodConfig->Forward = 0;
                methodConfig->IsDebug = false;
				methodConfig->IsOpen = true;
				methodConfig->Method = func;
				methodConfig->IsAsync = false;
				methodConfig->IsRecord = false;
				methodConfig->IsClient = false;
				methodConfig->NetName = "rpc";
				methodConfig->FullName = method;
				methodConfig->Service = service;
				methodConfig->Net = rpc::Net::Tcp;
				methodConfig->SendToClient = false;
				methodConfig->Proto = rpc::Porto::None;
			}
			this->mAllServices.insert(methodConfig->Service);

			value->Get("Async", methodConfig->IsAsync);
			value->Get("IsOpen", methodConfig->IsOpen);
			value->Get("Record", methodConfig->IsRecord);
			value->Get("Forward", methodConfig->Forward);
			value->Get("Timeout", methodConfig->Timeout);
			value->Get("Request", methodConfig->Request);
			value->Get("Response", methodConfig->Response);
			value->Get("IsDebug", methodConfig->IsDebug);
            value->Get("IsClient", methodConfig->IsClient);
			value->Get("SendToClient", methodConfig->SendToClient);
			ClusterConfig::Inst()->GetServerName(service, methodConfig->Server);
			this->mRpcMethodConfig.emplace(methodConfig->FullName, std::move(methodConfig));
		}
		return true;
    }

	constexpr int RPC_ALL = 0;
	constexpr int RPC_SERVER = 1;
	constexpr int RPC_CLIENT = 2;
	bool RpcConfig::GetMethodConfigs(std::vector<const RpcMethodConfig*>& configs, int type) const
	{
		for (auto iter = this->mRpcMethodConfig.begin(); iter != this->mRpcMethodConfig.end(); iter++)
		{
			const std::unique_ptr<RpcMethodConfig>& config = iter->second;
			switch (type)
			{
				case RPC_ALL:
					configs.emplace_back(config.get());
					break;
				case RPC_SERVER:
					if (!config->IsClient)
					{
						configs.emplace_back(config.get());
					}
					break;
				case RPC_CLIENT:
					if (config->IsClient)
					{
						configs.emplace_back(config.get());
					}
					break;
			}
		}
		return !configs.empty();
	}

	bool RpcConfig::GetMethodConfigs(const std::string& name, std::vector<const RpcMethodConfig*> & configs) const
	{
		for(auto iter = this->mRpcMethodConfig.begin(); iter != this->mRpcMethodConfig.end(); iter++)
		{
			const std::unique_ptr<RpcMethodConfig> & config = iter->second;
			if(config->Service == name)
			{
				configs.emplace_back(config.get());
			}
		}
		return !configs.empty();
	}

    bool RpcConfig::OnReLoadJson()
    {
        return true;
    }

	void RpcConfig::GetRpcMethods(std::vector<std::string>& methods) const
	{
		auto iter = this->mRpcMethodConfig.begin();
		for(; iter != this->mRpcMethodConfig.end(); iter++)
		{
			methods.emplace_back(iter->first);
		}
	}

    const RpcMethodConfig *RpcConfig::GetMethodConfig(const std::string &fullName) const
    {
        auto iter = this->mRpcMethodConfig.find(fullName);
        return iter != this->mRpcMethodConfig.end() ? iter->second.get() : nullptr;
    }
}

namespace joke
{
    bool HttpConfig::OnLoadJson()
    {
		std::vector<const char *> keys;
		if(this->GetKeys(keys) <= 0)
		{
			return false;
		}
		std::unique_ptr<json::r::Value> value;
		for(const char * key : keys)
		{
			if(!this->Get(key, value))
			{
				return false;
			}
			std::string url(key);
			std::unique_ptr<HttpMethodConfig> methodConfig = std::make_unique<HttpMethodConfig>();
			{
				std::string bindMethod;
				methodConfig->Auth = true;
				if(value->Get("Bind", bindMethod))
				{
					if(help::Str::Split(bindMethod,  '.', methodConfig->Service, methodConfig->Method) != 0)
					{
						LOG_ERROR("bind function : {}", bindMethod);
						return false;
					}
				}
				methodConfig->Path = url;
				methodConfig->Auth = true;
				methodConfig->Permission = 1;
				methodConfig->IsAsync = false;
				methodConfig->IsRecord = false;
				methodConfig->Limit = 1024 * 1024;
				value->Get("Type", methodConfig->Type);
				value->Get("Auth", methodConfig->Auth);
				value->Get("Desc", methodConfig->Desc);
				value->Get("Limit", methodConfig->Limit);
				value->Get("Lock", methodConfig->IsLock);
				value->Get("Async", methodConfig->IsAsync);
				value->Get("Record", methodConfig->IsRecord);
				value->Get("Permiss", methodConfig->Permission);
				value->Get("ContentType", methodConfig->Content);
				std::vector<std::string> headers;
				if(!methodConfig->Content.empty())
				{
					headers.emplace_back("Content-Type");
				}
				if(methodConfig->Auth)
				{
					headers.emplace_back("Authorization");
				}
				std::unique_ptr<json::r::Value> doc;
				if(value->Get("Request", doc))
				{
					methodConfig->Request = doc->ToString();
				}
				size_t index = 0;
				for(const std::string & header : headers)
				{
					index++;
					methodConfig->Headers += header;
					if (index != headers.size())
					{
						methodConfig->Headers += ", ";
					}
				}
                this->AddMethodConfig(std::move(methodConfig));
			}

		}
		return true;
    }


	bool HttpConfig::AddMethodConfig(std::unique_ptr<HttpMethodConfig> config)
	{
		auto iter = this->mMethodConfigs.find(config->Path);
		if(iter != this->mMethodConfigs.end())
		{
			return false;
		}
        if(config->Path.find('/') != 0)
        {
            config->Path.insert(0, "/");
        }
		this->mAllService.insert(config->Service);
       // printf("url = %s\n", config->Path.c_str());
		this->mMethodConfigs.emplace(config->Path, std::move(config));
		return true;
	}

    bool HttpConfig::OnReLoadJson()
    {
        return true;
    }

    const HttpMethodConfig *HttpConfig::GetMethodConfig(const std::string & path) const
    {
        auto iter = this->mMethodConfigs.find(path);
		return iter != this->mMethodConfigs.end() ? iter->second.get() : nullptr;
    }

	void HttpConfig::GetMethodList(std::vector<const HttpMethodConfig*>& configs, const std::string& url) const
	{
		auto iter = this->mMethodConfigs.begin();
		for (; iter != this->mMethodConfigs.end(); iter++)
		{
			const std::string& path = iter->second->Path;
			if (path == url || path.find(url) != std::string::npos)
			{
				configs.emplace_back(iter->second.get());
			}
		}
	}

	void HttpConfig::GetMethodConfigs(std::vector<const HttpMethodConfig*>& configs, const std::string& method) const
	{
		auto iter = this->mMethodConfigs.begin();
		for (; iter != this->mMethodConfigs.end(); iter++)
		{
			if(method.empty() || iter->second->Type == method)
			{
				configs.emplace_back(iter->second.get());
			}
		}
	}
}