#include "ServiceConfig.h"
#include"Rpc/Client/Message.h"
#include"Util/Tools/String.h"
#include"Cluster/Config/ClusterConfig.h"
#include "Util/File/FileHelper.h"

namespace acs
{

	void InterfaceConfig::OnReload()
	{
		auto iter = this->mFileTime.begin();
		for (; iter != this->mFileTime.end(); iter++)
		{
			const std::string& path = iter->first;
			long long lastTime = help::fs::GetLastWriteTime(path);
			if (iter->second != lastTime)
			{
				if (this->LoadConfig(path))
				{
					LOG_INFO("reload [{}] ok", path);
				}
				else
				{
					LOG_ERROR("reload [{}] fail", path);
				}
			}
		}
	}

	void InterfaceConfig::OnLoad(const std::string& path)
	{
		long long time = help::fs::GetLastWriteTime(path);
		this->mFileTime[path] = time;
	}
}

namespace acs
{
	constexpr int RPC_ALL = 0;
	constexpr int RPC_SERVER = 1;
	constexpr int RPC_CLIENT = 2;

	bool RpcConfig::OnLoadJson()
	{
		std::vector<const char*> keys;
		if (this->GetKeys(keys) <= 0)
		{
			return false;
		}
		std::unique_ptr<json::r::Value> value;
		for (const char* key: keys)
		{
			if (!this->Get(key, value))
			{
				return false;
			}
			std::string service, func;
			const std::string method(key);
			RpcMethodConfig* methodConfig = this->MakeConfig(method);
			{
				if (help::Str::Split(method, '.', service, func) != 0)
				{
					LOG_ERROR("rpc config : {}", method);
					return false;
				}
				methodConfig->Timeout = 0;
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
				methodConfig->Forward = rpc::Forward::Fixed;
			}
			this->mAllServices.insert(methodConfig->Service);

			std::unordered_map<std::string, int> NetMap{
					{ "tcp",   rpc::Net::Tcp },
					{ "udp",   rpc::Net::Udp },
					{ "kcp",   rpc::Net::Kcp },
					{ "Http",  rpc::Net::Http },
					{ "redis", rpc::Net::Redis },
			};

			std::unordered_map<std::string, int> ForwardMap{
					{ "hash",   rpc::Forward::Hash },
					{ "fixed",  rpc::Forward::Fixed },
					{ "random", rpc::Forward::Random },
			};

			std::string net, forward;
			if (value->Get("Net", net))
			{
				auto iter = NetMap.find(net);
				if (iter == NetMap.end())
				{
					return false;
				}
				methodConfig->Net = iter->second;
			}

			if (value->Get("Forward", forward))
			{
				auto iter = ForwardMap.find(net);
				if (iter == ForwardMap.end())
				{
					return false;
				}
				methodConfig->Forward = iter->second;
			}

			value->Get("Async", methodConfig->IsAsync);
			value->Get("IsOpen", methodConfig->IsOpen);
			value->Get("Record", methodConfig->IsRecord);
			value->Get("Timeout", methodConfig->Timeout);
			value->Get("Request", methodConfig->Request);
			value->Get("Response", methodConfig->Response);
			value->Get("IsDebug", methodConfig->IsDebug);
			value->Get("IsClient", methodConfig->IsClient);
			value->Get("SendToClient", methodConfig->SendToClient);
			ClusterConfig::Inst()->GetServerName(service, methodConfig->Server);
		}
		return true;
	}


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

	bool RpcConfig::GetMethodConfigs(const std::string& name, std::vector<const RpcMethodConfig*>& configs) const
	{
		for (auto iter = this->mRpcMethodConfig.begin(); iter != this->mRpcMethodConfig.end(); iter++)
		{
			const std::unique_ptr<RpcMethodConfig>& config = iter->second;
			if (config->Service == name)
			{
				configs.emplace_back(config.get());
			}
		}
		return !configs.empty();
	}

	bool RpcConfig::OnReLoadJson()
	{
		return this->OnLoadJson();
	}

	RpcMethodConfig* RpcConfig::MakeConfig(const std::string& name)
	{
		RpcMethodConfig* methodConfig = nullptr;
		auto iter = this->mRpcMethodConfig.find(name);
		if (iter == this->mRpcMethodConfig.end())
		{
			std::unique_ptr<RpcMethodConfig> config = std::make_unique<RpcMethodConfig>();
			{
				config->FullName = name;
				methodConfig = config.get();
				this->mRpcMethodConfig.emplace(name, std::move(config));
			}
		}
		else
		{
			methodConfig = iter->second.get();
		}
		return methodConfig;
	}

	void RpcConfig::GetRpcMethods(std::vector<std::string>& methods) const
	{
		auto iter = this->mRpcMethodConfig.begin();
		for (; iter != this->mRpcMethodConfig.end(); iter++)
		{
			methods.emplace_back(iter->first);
		}
	}

	const RpcMethodConfig* RpcConfig::GetMethodConfig(const std::string& fullName) const
	{
		auto iter = this->mRpcMethodConfig.find(fullName);
		return iter != this->mRpcMethodConfig.end() ? iter->second.get() : nullptr;
	}
}

namespace acs
{
	bool HttpConfig::OnLoadJson()
	{
		std::vector<const char*> keys;
		if (this->GetKeys(keys) <= 0)
		{
			return false;
		}
		std::unique_ptr<json::r::Value> value;
		for (const char* key: keys)
		{
			if (!this->Get(key, value))
			{
				return false;
			}
			std::string url(key);
			HttpMethodConfig* methodConfig = this->MakeConfig(url);
			{
				std::string bindMethod;
				if (value->Get("Bind", bindMethod))
				{
					if (help::Str::Split(bindMethod, '.', methodConfig->Service, methodConfig->Method) != 0)
					{
						LOG_ERROR("bind function : {}", bindMethod);
						return false;
					}
				}
				methodConfig->Auth = true;
				methodConfig->Open = true;
				methodConfig->Permission = 1;
				methodConfig->IsAsync = false;
				methodConfig->IsRecord = false;
				methodConfig->Limit = 1024 * 1024;
				methodConfig->Headers.clear();
				value->Get("Type", methodConfig->Type);
				value->Get("Auth", methodConfig->Auth);
				value->Get("Desc", methodConfig->Desc);
				value->Get("Limit", methodConfig->Limit);
				value->Get("Lock", methodConfig->IsLock);
				value->Get("Async", methodConfig->IsAsync);
				value->Get("Token", methodConfig->Token);
				value->Get("Record", methodConfig->IsRecord);
				value->Get("Permiss", methodConfig->Permission);
				value->Get("ContentType", methodConfig->Content);
				std::vector<std::string> headers;
				if (methodConfig->Content.empty() && methodConfig->Type == "POST")
				{
					headers.emplace_back("Content-Type");
					methodConfig->Content = "application/json";
				}
				if (!methodConfig->Content.empty())
				{
					headers.emplace_back("Content-Type");
				}
				if (methodConfig->Auth)
				{
					headers.emplace_back("Authorization");
				}
				std::unique_ptr<json::r::Value> jsonArray;
				if (value->Get("WhiteList", jsonArray))
				{
					size_t index = 0;
					std::string value;
					while (jsonArray->Get(index, value))
					{
						index++;
						methodConfig->WhiteList.emplace(value);
					}
				}
				std::unique_ptr<json::r::Value> doc;
				if (value->Get("Request", doc))
				{
					methodConfig->Request = doc->ToString();
				}
				size_t index = 0;
				for (const std::string& header: headers)
				{
					index++;
					methodConfig->Headers += header;
					if (index != headers.size())
					{
						methodConfig->Headers += ", ";
					}
				}
				this->mAllService.insert(methodConfig->Service);
			}
		}
		return true;
	}

	HttpMethodConfig* HttpConfig::MakeConfig(const std::string& url)
	{
		std::string path(url);
		if (path.find('/') != 0)
		{
			path.insert(0, "/");
		}
		HttpMethodConfig* methodConfig = nullptr;
		auto iter = this->mMethodConfigs.find(path);
		if (iter == this->mMethodConfigs.end())
		{
			std::unique_ptr<HttpMethodConfig> config = std::make_unique<HttpMethodConfig>();
			{
				config->Path = path;
				methodConfig = config.get();
				this->mMethodConfigs.emplace(path, std::move(config));
			}
		}
		else
		{
			methodConfig = iter->second.get();
		}
		return methodConfig;
	}

	bool HttpConfig::OnReLoadJson()
	{
		return this->OnLoadJson();
	}

	const HttpMethodConfig* HttpConfig::GetMethodConfig(const std::string& path) const
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
			if (method.empty() || iter->second->Type == method)
			{
				configs.emplace_back(iter->second.get());
			}
		}
	}
}