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
		this->mIsStartComplete = false;
		BIND_SUB_FUNCTION(LocalService::Add);
		BIND_SUB_FUNCTION(LocalService::Push);
		BIND_SUB_FUNCTION(LocalService::Remove);
		const ServerConfig& config = App::Get()->GetConfig();
		LOG_CHECK_RET_FALSE(config.GetMember("area_id", this->mAreaId));
		LOG_CHECK_RET_FALSE(config.GetMember("node_name", this->mNodeName));
		return true;
	}

	bool LocalService::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mRedisComponent = this->GetComponent<RedisComponent>());
		LOG_CHECK_RET_FALSE(this->mHttpComponent = this->GetComponent<HttpClientComponent>());

		TcpServerComponent * tcpServerComponent = this->GetComponent<TcpServerComponent>();
		LOG_CHECK_RET_FALSE(tcpServerComponent != nullptr);

		LOG_CHECK_RET_FALSE(tcpServerComponent->GetTcpConfig("rpc"));
		LOG_CHECK_RET_FALSE(tcpServerComponent->GetTcpConfig("http"));
		return true;
	}

	void LocalService::OnAddService(LocalServerRpc* component)
	{
		TcpServerComponent * tcpServerComponent = this->GetComponent<TcpServerComponent>();
		if(this->mIsStartComplete)
		{
			Json::Writer jsonWriter;
			jsonWriter.AddMember("area_id", this->mAreaId);
			jsonWriter.AddMember("service", component->GetName());
			jsonWriter.AddMember("address", tcpServerComponent->GetTcpConfig("rpc"));
			this->mRedisComponent->Publish("LocalService.Add", jsonWriter);
		}
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
		RpcServiceComponent * rpcServiceComponent = this->GetComponent<RpcServiceComponent>(service);
		if(rpcServiceComponent != nullptr)
		{
			rpcServiceComponent->AddRemoteAddress(address);
		}
	}

	void LocalService::RemoveByAddress(const std::string& address)
	{
		Json::Writer jsonWriter;
		jsonWriter.AddMember("address", address);
		this->mRedisComponent->Publish("LocalService.Remove", jsonWriter);
	}

	void LocalService::Push(const Json::Reader& jsonReader)
	{
		int areaId = 0;
		LOG_CHECK_RET(jsonReader.GetMember("area_id", areaId));
		if (areaId != 0 && areaId != this->mAreaId)
		{
			return;
		}
		std::string rpcAddress;
		std::string httpAddress;
		std::vector<std::string> services;
		LOG_CHECK_RET(jsonReader.GetMember("rpc", "service", services));
		LOG_CHECK_RET(jsonReader.GetMember("rpc", "address", rpcAddress));
		LOG_CHECK_RET(jsonReader.GetMember("http", "address", httpAddress));

		for(const std::string & service : services)
		{
			RpcServiceComponent* rpcServiceComponent = this->GetComponent<RpcServiceComponent>(service);
			if (rpcServiceComponent != nullptr)
			{
				rpcServiceComponent->AddRemoteAddress(rpcAddress);
			}
		}

		ElapsedTimer elapsedTimer;
		Json::Writer jsonWriter;
		this->GetServiceInfo(jsonWriter);
		string url = fmt::format("http://{0}/logic/service/push", httpAddress);
		std::shared_ptr<HttpAsyncResponse> httpResponse = this->mHttpComponent->Post(url, jsonWriter);
		if (httpResponse != nullptr)
		{
			LOG_INFO("post service to {0} successful {1}ms", httpAddress, elapsedTimer.GetMs());
		}
	}

	void LocalService::OnComplete()//通知其他服务器 我加入了
	{
		Json::Writer jsonWriter;
		this->mIsStartComplete = true;
		jsonWriter.AddMember("http", this->GetApp()->GetConfig().GetHttpAddress());
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

	bool LocalService::AddNewService(const std::string& name)
	{
		Component* component = this->GetComponent<Component>(name);
		if (component != nullptr)
		{
			return false;
		}
		component = ComponentFactory::CreateComponent(name);
		if (component == nullptr || dynamic_cast<RpcServiceComponent*>(component) == nullptr)
		{
			delete component;
			return false;
		}
		if (!this->mEntity->AddComponent(name, component) || component->LateAwake())
		{
			this->mEntity->RemoveComponent(name);
			return false;
		}
		LOG_INFO("start new component [", name, "] successful");
		return true;
	}

	void LocalService::Remove(const Json::Reader& jsonReader)
	{
		std::string address;
		LOG_CHECK_RET(jsonReader.GetMember("address", address));

		// TODO
//		RpcServiceComponent* rpcServiceComponent = this->GetComponent<RpcServiceComponent>(service);
//		if(rpcServiceComponent != nullptr)
//		{
//			rpcServiceComponent->DelRemoteAddress(address);
//		}
	}

	void LocalService::GetServiceInfo(Json::Writer& jsonWriter)
	{
		const ServerConfig & serverConfig = this->GetApp()->GetConfig();

		jsonWriter.StartObject("rpc");
		jsonWriter.AddMember("address", serverConfig.GetRpcAddress());

		std::vector<std::string> tempArray;
		std::list<LocalServerRpc*> rpcServices;
		App::Get()->GetTypeComponents(rpcServices);
		for (LocalServerRpc * rpcService : rpcServices)
		{
			tempArray.emplace_back(rpcService->GetName());
		}
		jsonWriter.AddMember("service", tempArray);
		jsonWriter.EndObject();

		jsonWriter.StartObject("http");
		jsonWriter.AddMember("address", serverConfig.GetHttpAddress());
		jsonWriter.EndObject();

		jsonWriter.AddMember("area_id", this->mAreaId);
		jsonWriter.AddMember("node_name", this->mNodeName);
	}
}// namespace Sentry