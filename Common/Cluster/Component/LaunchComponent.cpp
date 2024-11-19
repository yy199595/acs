//
// Created by zmhy0073 on 2022/10/12.
//

#include"LaunchComponent.h"
#include"Core/System/System.h"
#include"Util/File/FileHelper.h"
#include"Http/Component/HttpComponent.h"
#include"Cluster//Config/ClusterConfig.h"
#include"Rpc/Service/LuaRpcService.h"
#include"Http//Service/LuaHttpService.h"
#include"Lua/Component/LuaComponent.h"
#include"Rpc/Component/InnerNetComponent.h"
#include"Rpc/Component/DispatchComponent.h"
#include"Router/Component/RouterComponent.h"

namespace acs
{
	bool LaunchComponent::Awake()
	{
		std::unique_ptr<json::r::Value> luaObject;
		if (this->mApp->Config().Get("lua", luaObject))
		{
			this->mApp->AddComponent<LuaComponent>();
		}

		if (ClusterConfig::Inst() == nullptr)
		{
			return true;
		}
		const NodeConfig* nodeConfig = ClusterConfig::Inst()->GetConfig();
		if (nodeConfig == nullptr)
		{
			LOG_ERROR("{} config is null", ClusterConfig::Inst()->GetConfigName());
			return false;
		}
		std::vector<std::string> components;
		std::vector<std::string> rpcService;
		std::vector<std::string> httpService;
		nodeConfig->GetComponents(components);
		nodeConfig->GetRpcServices(rpcService);
		nodeConfig->GetHttpServices(httpService);
		LOG_CHECK_RET_FALSE(this->AddComponent(components));
		LOG_CHECK_RET_FALSE(this->AddRpcService(rpcService));
		LOG_CHECK_RET_FALSE(this->AddHttpService(httpService));
		LOG_CHECK_RET_FALSE(this->LoadListenConfig())

		return true;
	}

	bool LaunchComponent::AddRpcService(const std::vector<std::string>& service)
	{
		const RpcConfig* rpcConfig = RpcConfig::Inst();
		for (const std::string& name: service)
		{
			if (!rpcConfig->HasService(name))
			{
				LOG_ERROR("not rpc service => ", name);
				return false;
			}
			if (this->mApp->HasComponent(name))
			{
				continue;
			}
			if (!this->mApp->AddComponent(name))
			{
				std::unique_ptr<Component> component(new LuaRpcService());
				if (!this->mApp->AddComponent(name, std::move(component)))
				{
					LOG_ERROR("add rpc service [{}] error", name);
					return false;
				}
			}
		}
		return true;
	}

	bool LaunchComponent::AddHttpService(const std::vector<std::string>& service)
	{
		const HttpConfig* httpConfig = HttpConfig::Inst();
		for (const std::string& name: service)
		{
			if (this->mApp->HasComponent(name))
			{
				continue;
			}
			if (!httpConfig->HasService(name))
			{
				LOG_ERROR("not http service => {}", name);
				return false;
			}
			if (!this->mApp->AddComponent(name))
			{
				std::unique_ptr<Component> component(new LuaHttpService());
				if (!this->mApp->AddComponent(name, std::move(component)))
				{
					LOG_ERROR("add http service [{}] error", name);
					return false;
				}
			}
		}
		return true;
	}

	bool LaunchComponent::AddComponent(const std::vector<std::string>& components)
	{
		this->mApp->AddComponent<HttpComponent>();
		this->mApp->AddComponent<RouterComponent>();
		this->mApp->AddComponent<InnerNetComponent>();
		this->mApp->AddComponent<DispatchComponent>();
		for (const std::string& name: components)
		{
			if (this->mApp->HasComponent(name))
			{
				continue;
			}
			if (!this->mApp->AddComponent(name))
			{
				LOG_ERROR("add {} error", name);
				return false;
			}
		}
		return true;
	}

	bool LaunchComponent::LoadListenConfig()
	{
		std::vector<const char*> keys;
		std::unique_ptr<json::r::Value> jsonObj;
		const ServerConfig* config = ServerConfig::Inst();
		{
			LOG_CHECK_RET_FALSE(config->Get("core", jsonObj));
		}
		LOG_CHECK_RET_FALSE(config->Get("listen", jsonObj));

		std::unordered_map<std::string, int> ProtocolTypeMap = {
				{ "tcp", proto_type::tcp },
				{ "udp", proto_type::udp }
		};
		jsonObj->GetKeys(keys);
		for (const char* key: keys)
		{
			std::string value;
			std::unique_ptr<json::r::Value> jsonData;
			if (!jsonObj->Get(key, jsonData))
			{
				return false;
			}
			ListenConfig listenConfig;
			{
				listenConfig.Port = 0;
				listenConfig.Name = key;
				listenConfig.MaxConn = 0;
			}
			jsonData->Get("port", listenConfig.Port);
			jsonData->Get("max_conn", listenConfig.MaxConn);
			jsonData->Get("protocol", listenConfig.ProtoName);
			jsonData->Get("component", listenConfig.Component);
#ifdef __ENABLE_OPEN_SSL__
			jsonData->Get("key", listenConfig.Key);
			jsonData->Get("cert", listenConfig.Cert);
#endif
			auto iter = ProtocolTypeMap.find(listenConfig.ProtoName);
			if(iter == ProtocolTypeMap.end())
			{
				LOG_ERROR("not find proto:{}", listenConfig.ProtoName)
				return false;
			}
			listenConfig.ProtoType = iter->second;
			if (listenConfig.Port > 0 && !listenConfig.Component.empty())
			{
				const std::string& host = config->Host();
				listenConfig.Addr = fmt::format("{}://{}:{}", listenConfig.Name, host, listenConfig.Port);
				switch(listenConfig.ProtoType)
				{
					case proto_type::tcp:
					{
						std::unique_ptr<ListenerComponent> listenerComponent = std::make_unique<ListenerComponent>();
						{
							std::string name = fmt::format("{}:ListenComponent", listenConfig.Name);
							if (!this->mApp->AddComponent(name, std::move(listenerComponent)))
							{
								return false;
							}
						}
						break;
					}
					case proto_type::udp:
					{
						if(!this->mApp->HasComponent(listenConfig.Component))
						{
							this->mApp->AddComponent(listenConfig.Component);
						}
						break;
					}
				}
				this->mTcpListens.emplace_back(listenConfig);
			}
		}
		return true;
	}

	bool LaunchComponent::LateAwake()
	{
		for (const ListenConfig& listenConfig: this->mTcpListens)
		{
			INetListen* tcpListen = nullptr;
			switch(listenConfig.ProtoType)
			{
				case proto_type::tcp:
				{
					std::string name = fmt::format("{}:ListenComponent", listenConfig.Name);
					tcpListen = this->GetComponent<INetListen>(name);
					break;
				}
				case proto_type::udp:
				{
					tcpListen = this->GetComponent<INetListen>(listenConfig.Component);
					break;
				}
			}
			if (tcpListen == nullptr || !tcpListen->StartListen(listenConfig))
			{
				LOG_ERROR("({}) listen [{}] fail", listenConfig.ProtoName, listenConfig.Addr);
				return false;
			}
			this->mApp->AddListen(listenConfig.Name, listenConfig.Addr);
			LOG_INFO("({}) listen [{}] ok", listenConfig.ProtoName, listenConfig.Addr);
		}
		return true;
	}

	void LaunchComponent::OnDestroy()
	{
		for (const ListenConfig& listenConfig: this->mTcpListens)
		{
			INetListen* tcpListen = nullptr;
			switch(listenConfig.ProtoType)
			{
				case proto_type::tcp:
				{
					std::string name = fmt::format("{}:ListenComponent", listenConfig.Name);
					tcpListen = this->GetComponent<INetListen>(name);
					break;
				}
				case proto_type::udp:
				{
					tcpListen = this->GetComponent<INetListen>(listenConfig.Component);
					break;
				}
			}
			if(!tcpListen->StopListen())
			{
				LOG_ERROR("({}) close [{}] fail", listenConfig.ProtoName, listenConfig.Addr);
				continue;
			}
			LOG_INFO("({}) close [{}] ok", listenConfig.ProtoName, listenConfig.Addr);
		}
	}
}
