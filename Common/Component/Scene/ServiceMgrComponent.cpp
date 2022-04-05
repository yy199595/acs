#include"ServiceMgrComponent.h"
#include"Service/ServiceProxy.h"
#include"Component/Rpc/RpcConfigComponent.h"
namespace Sentry
{
	bool ServiceMgrComponent::Awake()
	{
		return true;
	}

	bool ServiceMgrComponent::LateAwake()
	{
		RpcConfigComponent* component = this->GetComponent<RpcConfigComponent>();
		if (component == nullptr)
		{
			return false;
		}
		std::vector<std::string> services;
		component->GetServices(services);
		for (const std::string& name : services)
		{
			std::shared_ptr<ServiceProxy> serviceEntity(new ServiceProxy(name));
			this->mServiceEntityMap.emplace(name, serviceEntity);
		}
		return true;
	}

	void ServiceMgrComponent::OnServiceExit(const std::string& address)
	{
		
	}

	void ServiceMgrComponent::OnServiceJoin(const std::string& name, const std::string& address)
	{
		std::shared_ptr<ServiceProxy> serviceProxy = this->GetServiceProxy(name);
		if(serviceProxy == nullptr)
		{
			LOG_ERROR("not find service : [{0}]", name);
			return;
		}
		serviceProxy->AddAddress(address);
	}

	std::shared_ptr<ServiceProxy> ServiceMgrComponent::GetServiceProxy(const std::string& name)
	{
		auto iter = this->mServiceEntityMap.find(name);
		return iter != this->mServiceEntityMap.end() ? iter->second : nullptr;
	}

	const std::string ServiceMgrComponent::QueryAddress(const string& name)
	{
		std::shared_ptr<ServiceProxy> serviceProxy = this->GetServiceProxy(name);
		if(serviceProxy == nullptr)
		{
			return std::string();
		}
		return serviceProxy->AllotAddress();
	}
}// namespace Sentry
