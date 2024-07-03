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

namespace joke
{
    bool LaunchComponent::Awake()
	{
		this->AddComponent();
		std::vector<std::string> components;
		LOG_CHECK_RET_FALSE(ClusterConfig::Inst());
		const NodeConfig* nodeConfig = ClusterConfig::Inst()->GetConfig();
		if (nodeConfig->GetComponents(components))
		{
			for (const std::string& name: components)
			{
				if (!this->mApp->HasComponent(name))
				{
					if (!this->mApp->AddComponent(name))
					{
						LOG_ERROR("add {} error", name);
						return false;
					}
				}
			}
		}
		components.clear();
		std::unique_ptr<json::r::Value> luaObject;
		if(this->mApp->Config().Get("lua", luaObject))
		{
			this->mApp->AddComponent<LuaComponent>();
		}
		if(this->mApp->Config().Get("listen", luaObject))
		{
			this->mApp->AddComponent<ListenerComponent>();
		}
		if (nodeConfig->GetServices(components) > 0)
		{
			for (const std::string& name: components)
			{
				if(!this->AddService(name))
				{
					LOG_ERROR("add service {} fail", name);
					return false;
				}
			}
		}
		return true;
	}

	bool LaunchComponent::AddService(const std::string& name)
	{
		if (this->mApp->HasComponent(name))
		{
			return true;
		}
		if (RpcConfig::Inst()->HasService(name))
		{
			//创建实体服务
			if (!this->mApp->AddComponent(name))
			{
				std::unique_ptr<Component> component(new LuaRpcService());
				if (!this->mApp->AddComponent(name, std::move(component)))
				{
					LOG_ERROR("add physical service [{}] error", name);
					return false;
				}
			}
			return true;
		}
		if (HttpConfig::Inst()->HasService(name))
		{
			if (!this->mApp->AddComponent(name))
			{
				std::unique_ptr<Component> component(new LuaHttpService());
				if (!this->mApp->AddComponent(name, std::move(component)))
				{
					LOG_ERROR("add http service [{}] error", name);
					return false;
				}
			}
			return true;
		}
		return false;
	}

	void LaunchComponent::AddComponent()
	{
		this->mApp->AddComponent<HttpComponent>();
		this->mApp->AddComponent<RouterComponent>();
		this->mApp->AddComponent<InnerNetComponent>();
		this->mApp->AddComponent<DispatchComponent>();
	}
}
