#include"RegistryService.h"
#include"App/App.h"
#include"Network/Listener/NetworkListener.h"
#include"Network/Http/HttpAsyncRequest.h"
#include"Component/HttpService/HttpService.h"
#include"Component/RpcService/LocalServiceComponent.h"
#include"Network/Listener/TcpServerComponent.h"
#include"Component/Redis/MainRedisComponent.h"
namespace Sentry
{
	bool RegistryService::OnStartService(ServiceMethodRegister& methodRegister)
	{
		methodRegister.Bind("Add", &RegistryService::Add);
		methodRegister.Bind("Del", &RegistryService::Del);
		methodRegister.Bind("Push", &RegistryService::Push);
		return true;
	}

	bool RegistryService::LateAwake()
	{
		LOG_CHECK_RET_FALSE(ServiceComponent::LateAwake());
		this->mRedisComponent = this->GetComponent<MainRedisComponent>();
		LOG_CHECK_RET_FALSE(this->GetConfig().GetMember("area_id", this->mAreaId));
		LOG_CHECK_RET_FALSE(this->GetConfig().GetMember("node_name", this->mNodeName));
		LOG_CHECK_RET_FALSE(this->GetConfig().GetListenerAddress("rpc", this->mRpcAddress));
		LOG_CHECK_RET_FALSE(this->GetConfig().GetListenerAddress("http", this->mHttpAddress));
		return true;
	}

	void RegistryService::OnAddService(Component* component)
	{
		HttpService * httpService = component->Cast<HttpService>();
		ServiceComponent * localService = component->Cast<ServiceComponent>();

		sub::Add::Request request;
		request.set_area_id(this->mAreaId);
		request.set_service(component->GetName());
		if(httpService != nullptr)
		{
			request.set_address(this->mHttpAddress);
		}
		else if(localService != nullptr)
		{
			request.set_address(this->mRpcAddress);
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
			ServiceComponent * localService = this->GetComponent<ServiceComponent>(service);
			if(localService == nullptr)
			{
				return XCode::CallServiceNotFound;
			}
			localService->OnAddAddress(request.rpc().address());
		}

		for(const std::string & service : request.http().service())
		{
			HttpService* httpService = this->GetComponent<HttpService>(service);
			if(httpService == nullptr)
			{
				return XCode::CallServiceNotFound;
			}
			httpService->OnAddAddress(request.http().address());
		}

		std::vector<Component *> components;
		this->GetApp()->GetComponents(components);
		for(Component * component : components)
		{
			HttpService * httpService = component->Cast<HttpService>();
			ServiceComponent * localService = component->Cast<ServiceComponent>();
			if(httpService != nullptr)
			{
				response.mutable_http()->add_service(httpService->GetName());
			}
			if(localService != nullptr)
			{
				response.mutable_rpc()->add_service(localService->GetName());
			}
		}
		const ListenConfig* rpcConfig = this->GetConfig().GetListen("rpc");
		const ListenConfig* httpConfig = this->GetConfig().GetListen("http");
		response.mutable_rpc()->set_address(rpcConfig->Address);
		response.mutable_http()->set_address(httpConfig->Address);
		return XCode::Successful;
	}

	void RegistryService::OnComplete()//通知其他服务器 我加入了
	{
		sub::Push::Request request;
		request.mutable_rpc()->set_address(this->mRpcAddress);
		request.mutable_http()->set_address(this->mHttpAddress);

		std::vector<Component *> components;
		this->GetApp()->GetComponents(components);
		for(Component * component : components)
		{
			HttpService * httpService = component->Cast<HttpService>();
			ServiceComponent * localService = component->Cast<ServiceComponent>();
			if(httpService != nullptr)
			{
				request.mutable_http()->add_service(component->GetName());
			}
			else if(localService != nullptr)
			{
				request.mutable_rpc()->add_service(component->GetName());
			}
		}
		std::vector<std::string> channels;
		this->mRedisComponent->GetAllAddress(channels);
		for(const std::string & address : channels)
		{
			std::shared_ptr<sub::Push::Response> response(new sub::Push::Response());
			if(this->Call(address, "Push", request, response) != XCode::Successful)
			{
				LOG_ERROR("[" << address << "] push all service failure");
				continue;
			}
			LOG_INFO("[" << address << "] push all service successful");
			for(const std::string & service : response->rpc().service())
			{
				IServiceBase * serviceBase = this->GetComponent<IServiceBase>(service);
				serviceBase->OnAddAddress(response->rpc().address());
			}
			for(const std::string & service : response->http().service())
			{
				IServiceBase * serviceBase = this->GetComponent<IServiceBase>(service);
				serviceBase->OnAddAddress(response->http().address());
			}
		}
	}

	void RegistryService::OnDestory()
	{

	}

	XCode RegistryService::Del(const sub::Del::Request& request)
	{
		IServiceBase * serviceBase = this->GetComponent<IServiceBase>(request.service());
		if(serviceBase != nullptr)
		{
			serviceBase->OnDelAddress(request.address());
			return XCode::Successful;
		}
		return XCode::Failure;
	}
}// namespace Sentry