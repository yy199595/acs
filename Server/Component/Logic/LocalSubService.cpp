#include"LocalSubService.h"
#include"App/App.h"
#include"Network/Http/HttpAsyncRequest.h"
#include"Component/RpcService/LocalServerRpc.h"
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
		this->mRpcAddress = this->GetApp()->GetConfig().GetRpcAddress();
		LOG_CHECK_RET_FALSE(this->GetApp()->GetConfig().GetMember("area_id", this->mAreaId));
		LOG_CHECK_RET_FALSE(this->GetApp()->GetConfig().GetMember("node_name", this->mNodeName));
		return true;
	}

	void LocalSubService::OnAddService(Component* component)
	{
		if(component->Cast<IServiceBase>() != nullptr)
		{
			Json::Writer jsonWriter;
			jsonWriter.AddMember("area_id", this->mAreaId);
			jsonWriter.AddMember("address", this->mRpcAddress);
			jsonWriter.AddMember("service", component->GetName());
			LOG_DEBUG("publish " << component->GetName() << " to all server");
			LOG_CHECK_RET(this->Publish("LocalSubService.Add", jsonWriter));
		}
	}

	void LocalSubService::OnDelService(Component* component)
	{

	}

	void LocalSubService::Add(const Json::Reader& jsonReader)
	{
		int areaId = 0;
		std::string address;
		std::string service;
		LOG_CHECK_RET(jsonReader.GetMember("area_id", areaId));
		LOG_CHECK_RET(jsonReader.GetMember("address", address));
		LOG_CHECK_RET(jsonReader.GetMember("service", service));
		if (areaId != 0 && areaId != this->mAreaId)
		{
			return;
		}
		IServiceBase * serviceBase = this->GetComponent<IServiceBase>(service);
		if(serviceBase != nullptr)
		{
			serviceBase->OnAddAddress(address);
		}
	}

	void LocalSubService::Push(const Json::Reader& jsonReader)
	{
		std::string address;
		std::vector<std::string> services;
		jsonReader.GetMember("rpc", address);
		jsonReader.GetMember("service", services);
		for(const std::string & service : services)
		{
			IServiceBase * serviceBase = this->GetComponent<IServiceBase>(service);
			if(serviceBase != nullptr)
			{
				serviceBase->OnAddAddress(address);
			}
		}

		services.clear();
		bool isResponse = false;
		if(jsonReader.GetMember("response", isResponse) && isResponse)
		{
			Json::Writer jsonWriter;
			std::vector<std::string> components;
			this->GetApp()->GetComponents(components);
			for(const std::string & name : components)
			{
				LocalServerRpc * localServerRpc = this->GetApp()->GetComponent<LocalServerRpc>(name);
				if(localServerRpc != nullptr && localServerRpc->IsStartService())
				{
					services.emplace_back(name);
				}
			}
			jsonWriter.AddMember("service", services);
			jsonWriter.AddMember("rpc", this->mRpcAddress);
			this->Publish(address, "LocalSubService.Push", jsonWriter);
		}
	}

	void LocalSubService::GetServiceInfo(Json::Writer& jsonWriter)
	{
		const ServerConfig & config = this->GetApp()->GetConfig();


		jsonWriter.StartObject("rpc");
		jsonWriter.AddMember("address", config.GetRpcAddress());

		jsonWriter.EndObject();

		jsonWriter.StartObject("http");
		jsonWriter.AddMember("address", config.GetHttpAddress());

		jsonWriter.EndObject();


		jsonWriter.StartArray("sub");

		jsonWriter.EndObject();
	}

	void LocalSubService::OnComplete()//通知其他服务器 我加入了
	{
		Json::Writer jsonWriter;
		jsonWriter.StartArray("service");
		std::vector<std::string> components;
		this->GetApp()->GetComponents(components);
		for(const std::string & name : components)
		{
			LocalServerRpc * localServerRpc = this->GetApp()->GetComponent<LocalServerRpc>(name);
			if(localServerRpc != nullptr && localServerRpc->IsStartService())
			{
				jsonWriter.AddMember(name);
			}
		}

		jsonWriter.EndArray();
		jsonWriter.AddMember("response", true);
		jsonWriter.AddMember("rpc", this->mRpcAddress);
		long long number = this->Publish("LocalSubService.Push", jsonWriter);
		LOG_DEBUG("publish successful count = " << number);
	}

	void LocalSubService::OnDestory()
	{
		std::string jsonContent;
		Json::Writer jsonWriter;
		jsonWriter.AddMember("area_id", this->mAreaId);
		jsonWriter.AddMember("address", this->mRpcAddress);
		LOG_DEBUG("remove this form count = " << this->Publish("LocalSubService.Remove", jsonWriter));

	}

	void LocalSubService::Remove(const Json::Reader& jsonReader)
	{
		std::string address;
		LOG_CHECK_RET(jsonReader.GetMember("address", address));

		// TODO
//		RpcServiceNode* rpcServiceComponent = this->GetComponent<RpcServiceNode>(service);
//		if(rpcServiceComponent != nullptr)
//		{
//			rpcServiceComponent->DelRemoteAddress(address);
//		}
	}
}// namespace Sentry