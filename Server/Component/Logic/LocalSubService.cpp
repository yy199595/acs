#include"LocalSubService.h"
#include"App/App.h"
#include"Network/Listener/NetworkListener.h"
#include"Network/Http/HttpAsyncRequest.h"
#include"Component/HttpService/HttpService.h"
#include"Component/RpcService/LocalServiceComponent.h"
#include"Network/Listener/TcpServerComponent.h"
#include"Component/Redis/MainRedisComponent.h"
namespace Sentry
{
	bool LocalSubService::OnInitService(SubServiceRegister& methodRegister)
	{
		methodRegister.Bind("Add", &LocalSubService::Add);
		methodRegister.Bind("Push", &LocalSubService::Push);
		methodRegister.Bind("Remove", &LocalSubService::Remove);
		return true;
	}

	bool LocalSubService::LateAwake()
	{
		LOG_CHECK_RET_FALSE(RedisSubService::LateAwake());
		this->mRedisComponent = this->GetComponent<MainRedisComponent>();
		LOG_CHECK_RET_FALSE(this->GetConfig().GetMember("area_id", this->mAreaId));
		LOG_CHECK_RET_FALSE(this->GetConfig().GetMember("node_name", this->mNodeName));
		LOG_CHECK_RET_FALSE(this->GetConfig().GetListenerAddress("rpc", this->mRpcAddress));
		LOG_CHECK_RET_FALSE(this->GetConfig().GetListenerAddress("http", this->mHttpAddress));
		return true;
	}

	void LocalSubService::OnAddService(Component* component)
	{
		HttpService * httpService = component->Cast<HttpService>();
		RedisSubService * redisSubService = component->Cast<RedisSubService>();
		LocalServiceComponent * localService = component->Cast<LocalServiceComponent>();

		sub::Add::Request request;
		request.set_area_id(this->mAreaId);
		request.set_service(component->GetName());
		if(httpService != nullptr)
		{
			request.set_address(this->mHttpAddress);
		}
		else if(redisSubService != nullptr)
		{
			request.set_address(this->mRpcAddress);
		}
		else if(localService != nullptr)
		{
			request.set_address(this->mRpcAddress);
		}
		//TODO
	}

	void LocalSubService::OnDelService(Component* component)
	{

	}

	XCode LocalSubService::Add(const sub::Add::Request& jsonReader)
	{
		return XCode::Successful;
	}

	XCode LocalSubService::Push(const sub::Push::Request& request, sub::Push::Response& response)
	{

		for(const std::string & service : request.rpc().service())
		{
			LocalServiceComponent * localService = this->GetComponent<LocalServiceComponent>(service);
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
			LocalServiceComponent * localService = component->Cast<LocalServiceComponent>();
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

	void LocalSubService::OnComplete()//通知其他服务器 我加入了
	{
		sub::Push::Request request;
		request.mutable_rpc()->set_address(this->mRpcAddress);
		request.mutable_http()->set_address(this->mHttpAddress);

		std::vector<Component *> components;
		this->GetApp()->GetComponents(components);
		for(Component * component : components)
		{
			HttpService * httpService = component->Cast<HttpService>();
			LocalServiceComponent * localService = component->Cast<LocalServiceComponent>();
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
		this->mRedisComponent->GetAllChannel(channels);
		for(const std::string & address : channels)
		{
			std::shared_ptr<sub::Push::Response> response(new sub::Push::Response());
			if(this->Call(address, "Push", request, response) == XCode::Successful)
			{
				LOG_FATAL("push all service successful");
			}
		}
	}

	void LocalSubService::OnDestory()
	{

	}

	XCode LocalSubService::Remove(const sub::Del::Request& jsonReader)
	{

	}
}// namespace Sentry