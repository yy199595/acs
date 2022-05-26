#include"RegistryService.h"
#include"App/App.h"
#include"Network/Listener/NetworkListener.h"
#include"Network/Http/HttpAsyncRequest.h"
#include"Component/HttpService/HttpService.h"
#include"Component/RpcService/ServiceCallComponent.h"
#include"Network/Listener/TcpServerComponent.h"
#include"Component/Redis/MainRedisComponent.h"
namespace Sentry
{

	bool RegistryService::LateAwake()
	{
		this->mRedisComponent = this->GetComponent<MainRedisComponent>();
		LOG_CHECK_RET_FALSE(this->GetConfig().GetMember("area_id", this->mAreaId));
		LOG_CHECK_RET_FALSE(this->GetConfig().GetMember("node_name", this->mNodeName));
		LOG_CHECK_RET_FALSE(this->GetConfig().GetListenerAddress("rpc", this->mRpcAddress));
		return true;
	}

	void RegistryService::OnAddService(Component* component)
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

	void RegistryService::OnDelService(Component* component)
	{

	}

	XCode RegistryService::Add(const sub::Add::Request& jsonReader)
	{
		return XCode::Successful;
	}

	XCode RegistryService::Push(const sub::Push::Request& request, sub::Push::Response& response)
	{
		for(const std::string & service : request.rpc().service())
		{
			ServiceCallComponent * localService = this->GetComponent<ServiceCallComponent>(service);
			if(localService == nullptr)
			{
				return XCode::CallServiceNotFound;
			}
			localService->AddAddress(request.rpc().address());
		}

		std::vector<Component *> components;
		this->GetApp()->GetComponents(components);
		for(Component * component : components)
		{
			LocalRpcServiceBase * localService = component->Cast<LocalRpcServiceBase>();
			if(localService != nullptr && localService->IsStartService())
			{
				response.mutable_rpc()->add_service(localService->GetName());
			}
		}
		response.mutable_rpc()->set_address(this->mRpcAddress);
		return XCode::Successful;
	}

	void RegistryService::OnComplete()//通知其他服务器 我加入了
	{
		sub::Push::Request request;
		request.mutable_rpc()->set_address(this->mRpcAddress);

		std::vector<Component*> components;
		this->GetApp()->GetComponents(components);
		for (Component* component: components)
		{
			LocalRpcServiceBase* localService = component->Cast<LocalRpcServiceBase>();
			if (localService != nullptr && localService->IsStartService())
			{
				request.mutable_rpc()->add_service(component->GetName());
			}
		}
		std::vector<std::string> channels;
		this->mRedisComponent->GetAllAddress(channels);
		for (size_t index = 0; index < channels.size(); index++)
		{
			const std::string& address = channels[index];
			std::shared_ptr<sub::Push::Response> response(new sub::Push::Response());
			if (this->Call(address, "Push", request, response) != XCode::Successful)
			{
				LOG_ERROR("[" << address << "] push all service failure");
				continue;
			}
			LOG_ERROR("[" << address << "] push all service successful index = " << index);
			for (const std::string& service: response->rpc().service())
			{
				ServiceCallComponent* serviceBase = this->GetComponent<ServiceCallComponent>(service);
				if (serviceBase != nullptr)
				{
					serviceBase->AddAddress(response->rpc().address());
					LOG_INFO(service << " add address [" << response->rpc().address() << "]");
				}
			}
		}
	}

	void RegistryService::OnDestory()
	{

	}

	XCode RegistryService::Del(const sub::Del::Request& request)
	{

		return XCode::Failure;
	}
}// namespace Sentry