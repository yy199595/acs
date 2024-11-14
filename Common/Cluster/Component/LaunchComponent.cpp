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
		if (this->mApp->Config().Get("listen", luaObject))
		{
			this->mApp->AddComponent<ListenerComponent>();
		}
		const NodeConfig* nodeConfig = ClusterConfig::Inst()->GetConfig();
		if(nodeConfig == nullptr)
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
		return true;
	}

	bool LaunchComponent::AddRpcService(const std::vector<std::string> & service)
	{
		const RpcConfig * rpcConfig = RpcConfig::Inst();
		for(const std::string & name : service)
		{
			if(!rpcConfig->HasService(name))
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

	bool LaunchComponent::AddHttpService(const std::vector<std::string> & service)
	{
		const HttpConfig * httpConfig = HttpConfig::Inst();
		for(const std::string & name : service)
		{
			if (this->mApp->HasComponent(name))
			{
				continue;
			}
			if(!httpConfig->HasService(name))
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

	bool LaunchComponent::AddComponent(const std::vector<std::string> & components)
	{
		this->mApp->AddComponent<HttpComponent>();
		this->mApp->AddComponent<RouterComponent>();
		this->mApp->AddComponent<InnerNetComponent>();
		this->mApp->AddComponent<DispatchComponent>();
		for (const std::string& name: components)
		{
			if(this->mApp->HasComponent(name))
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
}
