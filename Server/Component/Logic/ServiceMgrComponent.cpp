#include"ServiceMgrComponent.h"
#include"App/App.h"
#include"Network/Listener/NetworkListener.h"
#include"Network/Http/HttpAsyncRequest.h"

#include"Network/Listener/TcpServerComponent.h"
#include"Component/Redis/MainRedisComponent.h"
#include"Component/RpcService/LocalServiceComponent.h"
namespace Sentry
{

	bool ServiceMgrComponent::LateAwake()
	{
		this->mRedisComponent = this->GetComponent<MainRedisComponent>();
		LOG_CHECK_RET_FALSE(this->GetConfig().GetMember("area_id", this->mAreaId));
		LOG_CHECK_RET_FALSE(this->GetConfig().GetMember("node_name", this->mNodeName));
		LOG_CHECK_RET_FALSE(this->GetConfig().GetListenerAddress("rpc", this->mRpcAddress));
		return true;
	}

	bool ServiceMgrComponent::OnRegisterEvent(NetEventRegistry& eventRegister)
	{
		eventRegister.Sub("service_add", &ServiceMgrComponent::OnServiceAdd, this);
		eventRegister.Sub("service_del", &ServiceMgrComponent::OnServiceDel, this);
		eventRegister.Sub("node_register", &ServiceMgrComponent::OnNodeRegister, this);
		return true;
	}

	void ServiceMgrComponent::OnAddService(Component* component)
	{
		if(component->Cast<ServiceComponent>())
		{
			Json::Writer json;
			json.AddMember("address", this->mRpcAddress);
			json.AddMember("service", component->GetName());
			if(!this->mRedisComponent->CallLua("node.add", json))
			{
				LOG_ERROR("call node.add failure " << component->GetName());
			}
		}
	}

	void ServiceMgrComponent::OnDelService(Component* component)
	{

	}

	void ServiceMgrComponent::OnComplete()//通知其他服务器 我加入了
	{
		Json::Writer json;
		json.StartArray("services");
		std::vector<Component*> components;
		this->GetApp()->GetComponents(components);
		for (Component* component: components)
		{
			LocalRpcService* localRpcServiceBase = component->Cast<LocalRpcService>();
			if (localRpcServiceBase != nullptr && localRpcServiceBase->IsStartService())
			{
				json.AddMember(component->GetName());
			}
		}
		json.EndArray();
		json.AddMember("address", this->mRpcAddress);
		if(!this->mRedisComponent->CallLua("node.register", json))
		{
			LOG_ERROR("register failure");
			return;
		}
		this->RefreshService();
	}

	bool ServiceMgrComponent::OnNodeRegister(const Json::Reader& json)
	{
		std::string address;
		std::vector<std::string> services;
		LOG_CHECK_RET_FALSE(json.GetMember("address", address));
		LOG_CHECK_RET_FALSE(json.GetMember("services", services));
		for(const std::string & service : services)
		{
			LocalRpcService * localRpcService = this->GetComponent<LocalRpcService>(service);
			if(localRpcService != nullptr)
			{
				localRpcService->GetAddressProxy().AddAddress(address);
			}
		}
		return true;
	}

	bool ServiceMgrComponent::OnServiceAdd(const Json::Reader& json)
	{
		std::string address, service;
		LOG_CHECK_RET_FALSE(json.GetMember("address", address));
		LOG_CHECK_RET_FALSE(json.GetMember("service", service));
		LocalRpcService * localRpcService = this->GetComponent<LocalRpcService>(service);
		if(localRpcService != nullptr)
		{
			localRpcService->GetAddressProxy().AddAddress(address);
			return true;
		}
		return false;
	}

	bool ServiceMgrComponent::OnServiceDel(const Json::Reader& json)
	{
		std::string address, service;
		LOG_CHECK_RET_FALSE(json.GetMember("address", address));
		LOG_CHECK_RET_FALSE(json.GetMember("service", service));
		LocalRpcService* localRpcService = this->GetComponent<LocalRpcService>(service);
		return localRpcService != nullptr && localRpcService->GetAddressProxy().DelAddress(address);
	}

	bool ServiceMgrComponent::RefreshService()
	{
		Json::Writer json;
		json.StartArray("services");
		std::vector<Component*> components;
		this->GetApp()->GetComponents(components);
		for (Component* component : components)
		{
			LocalRpcService* localRpcServiceBase = component->Cast<LocalRpcService>();
			if (localRpcServiceBase != nullptr)
			{
				json.AddMember(component->GetName());
			}
		}
		json.EndArray();
		json.AddMember("address", this->mRpcAddress);
		std::shared_ptr<Json::Reader> response(new Json::Reader());
		if (!this->mRedisComponent->CallLua("node.refresh", json, response))
		{
			return false;
		}
		this->mJsonMessages.clear();
		response->GetMember("services", this->mJsonMessages);
		for(const std::string & jsonMessage : this->mJsonMessages)
		{
			Json::Reader jsonReader;
			if (jsonReader.ParseJson(jsonMessage))
			{
				std::string address;
				this->mServices.clear();
				jsonReader.GetMember("address", address);
				jsonReader.GetMember("services", this->mServices);
				for (const std::string& service : this->mServices)
				{
					LocalRpcService* localRpcService = this->GetApp()->GetComponent<LocalRpcService>(service);
					if (localRpcService != nullptr)
					{
						localRpcService->GetAddressProxy().AddAddress(address);
					}
				}
			}
		}
		return true;
	}

	void ServiceMgrComponent::OnDestory()
	{

	}
}// namespace Sentry