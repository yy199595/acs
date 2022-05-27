#include"ServiceMgrComponent.h"
#include"App/App.h"
#include"Network/Listener/NetworkListener.h"
#include"Network/Http/HttpAsyncRequest.h"
#include"Component/HttpService/HttpService.h"

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

	void ServiceMgrComponent::OnAddService(Component* component)
	{
		ServiceCallComponent * localService = component->Cast<ServiceCallComponent>();
		if(localService != nullptr)
		{
			sub::Add::Request request;
			request.set_area_id(this->mAreaId);
			request.set_address(this->mRpcAddress);
			request.set_service(component->GetName());
		}
		//TODO
	}

	void ServiceMgrComponent::OnDelService(Component* component)
	{

	}

	void ServiceMgrComponent::OnComplete()//通知其他服务器 我加入了
	{
		Json::Writer json;
		json.StartArray("");
		std::vector<Component*> components;
		this->GetApp()->GetComponents(components);
		for (Component* component: components)
		{
			LocalRpcServiceBase* localService = component->Cast<LocalRpcServiceBase>();
			if (localService != nullptr && localService->IsStartService())
			{
				json.AddMember(component->GetName());
			}
		}
		json.EndArray();
		json.AddMember("address", this->mRpcAddress);
		std::shared_ptr<Json::Reader> response(new Json::Reader());
		if(this->mRedisComponent->CallLua("node.add", json, response))
		{
			response->GetMember("")
		}
	}

	void ServiceMgrComponent::OnDestory()
	{

	}
}// namespace Sentry