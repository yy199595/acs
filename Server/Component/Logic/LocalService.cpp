#include"LocalService.h"
#include"App/App.h"
#include"Component/RpcService/LocalServerRpc.h"
#include"Network/Listener/NetworkListener.h"
#include"Network/Listener/TcpServerComponent.h"
#include"Component/Redis/RedisComponent.h"
#include"Component/Http/HttpClientComponent.h"
#include"Network/Http/HttpAsyncRequest.h"
namespace Sentry
{
	bool LocalService::Awake()
	{
		BIND_SUB_FUNCTION(LocalService::Add);
		BIND_SUB_FUNCTION(LocalService::Push);
		BIND_SUB_FUNCTION(LocalService::Remove);
		this->mRpcAddress = this->GetApp()->GetConfig().GetRpcAddress();
		LOG_CHECK_RET_FALSE(this->GetApp()->GetConfig().GetMember("area_id", this->mAreaId));
		LOG_CHECK_RET_FALSE(this->GetApp()->GetConfig().GetMember("node_name", this->mNodeName));
		return true;
	}

	bool LocalService::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mRedisComponent = this->GetComponent<RedisComponent>());
		LOG_CHECK_RET_FALSE(this->mHttpComponent = this->GetComponent<HttpClientComponent>());
		return true;
	}

	void LocalService::OnAddService(LocalServerRpc* component)
	{
		Json::Writer jsonWriter;
		jsonWriter.AddMember("area_id", this->mAreaId);
		jsonWriter.AddMember("address", this->mRpcAddress);
		jsonWriter.AddMember("service", component->GetName());
		this->mRedisComponent->Publish("LocalService.Add", jsonWriter);
	}

	void LocalService::OnDelService(LocalServerRpc* component)
	{

	}

	void LocalService::Add(const Json::Reader& jsonReader)
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
		RpcServiceNode * rpcServiceComponent = this->GetComponent<RpcServiceNode>(service);
		if(rpcServiceComponent != nullptr)
		{
			rpcServiceComponent->AddAddress(address);
		}
	}

	void LocalService::Push(const Json::Reader& jsonReader)
	{
		std::string address;
		std::vector<std::string> nodeServices;
		jsonReader.GetMember("rpc", address);
		jsonReader.GetMember("service", nodeServices);
		for(const std::string & service : nodeServices)
		{
			RpcServiceNode * rpcServiceNode = this->GetComponent<RpcServiceNode>(service);
			if(rpcServiceNode == nullptr)
			{
				rpcServiceNode = new RpcServiceNode();
				this->mEntity->AddComponent(service, rpcServiceNode);
			}
			rpcServiceNode->AddAddress(address);
		}

		bool isResponse = false;
		if(jsonReader.GetMember("response", isResponse) && isResponse)
		{
			nodeServices.clear();
			Json::Writer jsonWriter;
			std::vector<std::string> components;
			this->GetApp()->GetComponents(components);
			for(const std::string & name : components)
			{
				LocalServerRpc * localServerRpc = this->GetApp()->GetComponent<LocalServerRpc>(name);
				if(localServerRpc != nullptr)
				{
					nodeServices.emplace_back(name);
				}
			}
			jsonWriter.AddMember("service", nodeServices);
			this->mRedisComponent->Publish(address, "LocalService.Push", jsonWriter);
		}
	}

	void LocalService::GetServiceInfo(Json::Writer& jsonWriter)
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

	void LocalService::OnComplete()//通知其他服务器 我加入了
	{
		Json::Writer jsonWriter;
		std::vector<std::string> tempArray;
		std::vector<std::string> components;
		this->GetApp()->GetComponents(components);
		for(const std::string & name : components)
		{
			LocalServerRpc * localServerRpc = this->GetApp()->GetComponent<LocalServerRpc>(name);
			if(localServerRpc != nullptr)
			{
				tempArray.emplace_back(name);
			}
		}
		const std::string & address = this->GetApp()->GetConfig().GetRpcAddress();

		jsonWriter.AddMember("rpc", address);
		jsonWriter.AddMember("response", true);
		jsonWriter.AddMember("service", tempArray);
		long long number = this->mRedisComponent->Publish("LocalService.Push", jsonWriter);
		LOG_DEBUG("publish successful count = {0}", number);
	}

	void LocalService::OnDestory()
	{
		std::string jsonContent;
		Json::Writer jsonWriter;
		TcpServerComponent * tcpServerComponent = this->GetComponent<TcpServerComponent>();
		const std::string address = tcpServerComponent->GetTcpAddress("rpc");

		jsonWriter.AddMember("address", address);
		jsonWriter.AddMember("area_id", this->mAreaId);
		LOG_DEBUG("remove this form count = ", this->mRedisComponent->Publish("LocalService.Remove", jsonWriter));

	}

	void LocalService::Remove(const Json::Reader& jsonReader)
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