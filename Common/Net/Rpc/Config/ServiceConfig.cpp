#include "ServiceConfig.h"
#include"Rpc/Common/Message.h"
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

	RpcConfig::RpcConfig()
			: InterfaceConfig("RpcConfig")
	{

	}

	bool RpcConfig::OnLoadJson()
	{
		std::vector<const char*> keys;
		if (this->GetKeys(keys) <= 0)
		{
			return false;
		}
		std::unique_ptr<json::r::Value> value;

		std::unordered_map<std::string, char> NetMap{
				{ "ws",   rpc::Net::Ws },
				{ "tcp",   rpc::Net::Tcp },
				{ "udp",   rpc::Net::Udp },
				{ "kcp",   rpc::Net::Kcp },
				{ "http",  rpc::Net::Http },
				{ "redis", rpc::Net::Redis },
		};

		std::unordered_map<std::string, char> ForwardMap{
				{ "hash",   rpc::Forward::Hash },
				{ "fixed",  rpc::Forward::Fixed },
				{ "random", rpc::Forward::Random },
		};

		std::unordered_map<std::string, char> ProtoMap{
				{ "lua",     rpc::Porto::Lua },
				{ "pb", rpc::Porto::Protobuf },
				{ "json",     rpc::Porto::Json },
				{ "string",   rpc::Porto::String },
		};

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
				methodConfig->timeout = 0;
				methodConfig->debug = false;
				methodConfig->open = true;
				methodConfig->auth = true;
				methodConfig->method = func;
				methodConfig->async = false;
				methodConfig->record = false;
				methodConfig->client = false;
				methodConfig->NetName = "rpc";
				methodConfig->fullname = method;
				methodConfig->service = service;
				methodConfig->net = rpc::Net::Tcp;
				methodConfig->to_client = false;
				methodConfig->ProtoName = "pb";
				methodConfig->proto = rpc::Porto::Protobuf;
				methodConfig->forward = rpc::Forward::Fixed;
			}
			this->mAllServices.insert(methodConfig->service);

			if (value->Get("net", methodConfig->NetName))
			{
				auto iter = NetMap.find(methodConfig->NetName);
				if (iter == NetMap.end())
				{
					return false;
				}
				methodConfig->net = iter->second;
			}

			if (value->Get("forward", methodConfig->ForwardName))
			{
				auto iter = ForwardMap.find(methodConfig->ForwardName);
				if (iter == ForwardMap.end())
				{
					return false;
				}
				methodConfig->forward = iter->second;
			}

			if (value->Get("proto", methodConfig->ProtoName))
			{
				auto iter = ProtoMap.find(methodConfig->ProtoName);
				if (iter == ProtoMap.end())
				{
					return false;
				}
				methodConfig->proto = iter->second;
			}

			if (value->Get("request", methodConfig->request))
			{
				methodConfig->proto = rpc::Porto::Protobuf;
			}
			value->Get("auth", methodConfig->auth);
			value->Get("async", methodConfig->async);
			value->Get("open", methodConfig->open);
			value->Get("record", methodConfig->record);
			value->Get("timeout", methodConfig->timeout);
			value->Get("response", methodConfig->response);
			value->Get("debug", methodConfig->debug);
			value->Get("client", methodConfig->client);
			value->Get("to_client", methodConfig->to_client);

			ClusterConfig::Inst()->GetServerName(service, methodConfig->server);
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
					if (!config->client)
					{
						configs.emplace_back(config.get());
					}
					break;
				case RPC_CLIENT:
					if (config->client)
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
			if (config->service == name)
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
				config->fullname = name;
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
			std::string url(key);
			LOG_CHECK_RET_FALSE(this->Get(key, value));
			HttpMethodConfig* methodConfig = this->MakeConfig(url);
			{
				std::string bindMethod;
				LOG_CHECK_RET_FALSE(value->Get("bind", bindMethod));
				if (help::Str::Split(bindMethod, '.', methodConfig->service, methodConfig->method) != 0)
				{
					LOG_ERROR("bind function : {}", bindMethod);
					return false;
				}
				methodConfig->auth = true;
				methodConfig->open = true;
				methodConfig->permission = 1;
				methodConfig->async = false;
				methodConfig->record = false;
				methodConfig->limit = 1024 * 1024;
				methodConfig->headers.clear();
				methodConfig->WhiteList.clear();
				value->Get("type", methodConfig->type);
				value->Get("auth", methodConfig->auth);
				value->Get("desc", methodConfig->desc);
				value->Get("limit", methodConfig->limit);
				value->Get("lock", methodConfig->lock);
				value->Get("async", methodConfig->async);
				value->Get("token", methodConfig->token);
				value->Get("record", methodConfig->record);
				value->Get("permission", methodConfig->permission);
				value->Get("content-type", methodConfig->content);
				std::vector<std::string> headers;

				std::vector<std::string> whiteList;
				value->Get("white-list", whiteList);

				for(const std::string & ip : whiteList)
				{
					methodConfig->WhiteList.emplace(ip);
				}

				if (!methodConfig->content.empty())
				{
					headers.emplace_back("Content-Type");
				}
				if (methodConfig->auth)
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
				if (value->Get("request", doc))
				{
					methodConfig->request = doc->ToString();
				}
				size_t index = 0;
				for (const std::string& header: headers)
				{
					index++;
					methodConfig->headers += header;
					if (index != headers.size())
					{
						methodConfig->headers += ", ";
					}
				}
				this->mAllService.insert(methodConfig->service);
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
				config->path = path;
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
			const std::string& path = iter->second->path;
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
			if (method.empty() || iter->second->type == method)
			{
				configs.emplace_back(iter->second.get());
			}
		}
	}
}