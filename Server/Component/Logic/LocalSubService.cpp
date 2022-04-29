#include"LocalSubService.h"
#include"App/App.h"
#include"Network/Listener/NetworkListener.h"
#include"Network/Http/HttpAsyncRequest.h"
#include"Component/HttpService/HttpService.h"
#include"Component/RpcService/LocalServiceComponent.h"
#include"Network/Listener/TcpServerComponent.h"
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
		LOG_CHECK_RET_FALSE(SubService::LateAwake());
		LOG_CHECK_RET_FALSE(this->GetConfig().GetMember("area_id", this->mAreaId));
		LOG_CHECK_RET_FALSE(this->GetConfig().GetMember("node_name", this->mNodeName));
		return true;
	}

	void LocalSubService::OnAddService(Component* component)
	{
		if(component->Cast<LocalServiceComponent>() != nullptr)
		{
			std::string address;
			if(this->GetConfig().GetListenerAddress("rpc", address))
			{
				Json::Writer jsonWriter;
				jsonWriter.AddMember("address", address);
				jsonWriter.AddMember("area_id", this->mAreaId);
				jsonWriter.AddMember("service", component->GetName());
				LOG_CHECK_RET(this->Publish("Add", jsonWriter));
				LOG_DEBUG("publish " << component->GetName() << " to all server");
			}
		}
		else if(component->Cast<HttpService>() != nullptr)
		{
			std::string address;
			if(this->GetConfig().GetListenerAddress("http", address))
			{
				Json::Writer jsonWriter;
				jsonWriter.AddMember("address", address);
				jsonWriter.AddMember("area_id", this->mAreaId);
				jsonWriter.AddMember("service", component->GetName());
				LOG_CHECK_RET(this->Publish("Add", jsonWriter));
				LOG_DEBUG("publish " << component->GetName() << " to all server");
			}
		}
	}

	void LocalSubService::OnDelService(Component* component)
	{

	}

	XCode LocalSubService::Add(const Json::Reader& jsonReader)
	{
		int areaId = 0;
		std::string address, service;
		LOGIC_THROW_ERROR(jsonReader.GetMember("area_id", areaId));
		LOGIC_THROW_ERROR(jsonReader.GetMember("address", address));
		LOGIC_THROW_ERROR(jsonReader.GetMember("service", service));
		if (areaId != 0 && areaId != this->mAreaId)
		{
			return XCode::Successful;
		}
		IServiceBase * serviceBase = this->GetComponent<IServiceBase>(service);
		if(serviceBase != nullptr)
		{
			serviceBase->OnAddAddress(address);
		}
		return XCode::Successful;
	}

	XCode LocalSubService::Push(const Json::Reader& jsonReader, Json::Writer & response)
	{
		std::string httpToken;
		std::string rpcAddress;
		std::string httpAddress;
		std::vector<std::string> services;
		LOGIC_THROW_ERROR(jsonReader.GetMember("service", services));
		LOGIC_THROW_ERROR(jsonReader.GetMember("http", "token", httpToken));
		LOGIC_THROW_ERROR(jsonReader.GetMember("rpc", "address", rpcAddress));
		LOGIC_THROW_ERROR(jsonReader.GetMember("http", "address", httpAddress));

		for (const std::string& service: services)
		{
			Component* component = this->GetByName(service);
			if (component != nullptr)
			{
				HttpService* httpService = component->Cast<HttpService>();
				LocalServiceComponent* localServerRpc = component->Cast<LocalServiceComponent>();
				if (httpService != nullptr && !httpAddress.empty())
				{
					httpService->OnAddAddress(httpAddress);
				}
				if (localServerRpc != nullptr && !rpcAddress.empty())
				{
					localServerRpc->OnAddAddress(rpcAddress);
				}
			}
		}

		services.clear();
		std::vector<std::string> components;
		this->GetApp()->GetComponents(components);
		for (const std::string& name: components)
		{
			LocalServiceComponent* localServerRpc = this->GetComponent<LocalServiceComponent>(name);
			if (localServerRpc != nullptr && localServerRpc->IsStartService())
			{
				services.emplace_back(name);
			}
		}
		const ListenConfig* rpcConfig = this->GetConfig().GetListen("rpc");

		response.StartObject("rpc");
		response.AddMember("token", rpcConfig->Token);
		response.AddMember("address", rpcConfig->Address);
		response.EndObject();
		response.AddMember("service", services);
	}

	void LocalSubService::OnComplete()//通知其他服务器 我加入了
	{

		Json::Writer jsonWriter;
		jsonWriter.StartArray("service");
		std::vector<std::string> components;
		this->GetApp()->GetComponents(components);
		for(const std::string & name : components)
		{
			IServiceBase * localService = this->GetApp()->GetComponent<IServiceBase>(name);
			if(localService != nullptr && localService->IsStartService())
			{
				jsonWriter.AddMember(name);
			}
		}
		jsonWriter.EndArray();
		const ListenConfig * rpcConfig = this->GetConfig().GetListen("rpc");
		const ListenConfig * httpConfig = this->GetConfig().GetListen("http");
		if(rpcConfig != nullptr)
		{
			jsonWriter.StartObject("rpc");
			jsonWriter.AddMember("token", rpcConfig->Token);
			jsonWriter.AddMember("address", rpcConfig->Address);
			jsonWriter.EndObject();
		}

		if(httpConfig != nullptr)
		{
			jsonWriter.StartObject("http");
			jsonWriter.AddMember("token", httpConfig->Token);
			jsonWriter.AddMember("address", httpConfig->Address);
			jsonWriter.EndObject();
		}

		jsonWriter.AddMember("response", true);
		long long number = this->Publish("Push", jsonWriter);
		LOG_DEBUG("publish successful count = " << number);
	}

	void LocalSubService::OnDestory()
	{
		std::string address;
		Json::Writer jsonWriter;
		if(this->GetConfig().GetListenerAddress("rpc", address))
		{
			jsonWriter.AddMember("address", address);
			jsonWriter.AddMember("area_id", this->mAreaId);
			LOG_DEBUG("remove this form count = " << this->Publish("Remove", jsonWriter));
		}
	}

	XCode LocalSubService::Remove(const Json::Reader& jsonReader)
	{
		std::string address;
		LOGIC_THROW_ERROR(jsonReader.GetMember("address", address));

		// TODO
//		RemoteServiceComponent* rpcServiceComponent = this->GetComponent<RemoteServiceComponent>(service);
//		if(rpcServiceComponent != nullptr)
//		{
//			rpcServiceComponent->DelRemoteAddress(address);
//		}
	}
}// namespace Sentry