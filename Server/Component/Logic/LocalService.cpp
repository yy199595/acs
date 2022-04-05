#include"LocalService.h"
#include"App/App.h"
#include"Component/Service/RpcService.h"
#include"Service/ServiceProxy.h"
#include"Component/Scene/ServiceMgrComponent.h"
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
		ServiceMgrComponent * serviceMgrComponent = this->GetComponent<ServiceMgrComponent>();
		LOG_CHECK_RET_FALSE(tcpServerComponent != nullptr && serviceMgrComponent != nullptr);

		LOG_CHECK_RET_FALSE(tcpServerComponent->GetTcpConfig("rpc"));
		LOG_CHECK_RET_FALSE(tcpServerComponent->GetTcpConfig("http"));

		std::vector<Component *> components;
		this->GetApp()->GetComponents(components);
		for(Component * component : components)
		{
			IRemoteService * serviceListen = component->Cast<IRemoteService>();
			if(serviceListen != nullptr)
			{
				this->mRemoteListeners.emplace_back(serviceListen);
			}
		}
		return !this->mRemoteListeners.empty();
	}

	void LocalService::OnAddService(RpcService* component)
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

	void LocalService::OnDelService(RpcService* component)
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
		for(IRemoteService * serviceListen : this->mRemoteListeners)
		{
			serviceListen->OnServiceJoin(service, address);
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

		for(IRemoteService * serviceListen : this->mRemoteListeners)
		{
			for(const std::string & service : services)
			{
				serviceListen->OnServiceJoin(service, rpcAddress);
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

	void LocalService::OnStart() //通知其他服务器 我加入了
	{
		Json::Writer jsonWriter;
		this->mIsStartComplete = true;
		this->GetServiceInfo(jsonWriter);
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
		if (component == nullptr || dynamic_cast<RpcService*>(component) == nullptr)
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
		for(IRemoteService * serviceListen : this->mRemoteListeners)
		{
			serviceListen->OnServiceExit(address);
		}
	}

	void LocalService::GetServiceInfo(Json::Writer& jsonWriter)
	{
		TcpServerComponent * tcpServerComponent = this->GetComponent<TcpServerComponent>();
		const std::string rpcAddress = tcpServerComponent->GetTcpAddress("rpc");
		const std::string httpAddress = tcpServerComponent->GetTcpAddress("http");

		jsonWriter.StartObject("rpc");
		jsonWriter.AddMember("address", rpcAddress);

		std::vector<std::string> tempArray;
		std::list<RpcService*> rpcServices;
		App::Get()->GetTypeComponents(rpcServices);
		for (RpcService* rpcService : rpcServices)
		{
			tempArray.emplace_back(rpcService->GetName());
		}
		jsonWriter.AddMember("service", tempArray);
		jsonWriter.EndObject();

		jsonWriter.StartObject("http");
		jsonWriter.AddMember("address", httpAddress);
		jsonWriter.EndObject();

		jsonWriter.AddMember("area_id", this->mAreaId);
		jsonWriter.AddMember("node_name", this->mNodeName);
	}
}// namespace Sentry