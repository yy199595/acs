#pragma once
#include"Json/JsonWriter.h"
#include"Component/Service/SubService.h"
using namespace com;
namespace Sentry
{
	class LocalServerRpc;

	class RpcComponent;

	class TaskComponent;

	class ServiceMgrComponent;

	class LocalService : public SubService, public IStart, public ILocalService<LocalServerRpc>
	{
	 public:
		LocalService() = default;
		~LocalService() override = default;

	 public:
		bool Awake() final;
		bool LateAwake() final;
		void OnStart() final;
		void OnDestory() final;
	 public:
		bool AddNewService(const std::string& service);
		void RemoveByAddress(const std::string& address);
	 protected:
		void OnAddService(LocalServerRpc *component) final;
		void OnDelService(LocalServerRpc *component) final;
	 private:
		void Add(const Json::Reader& jsonReader);
		void Push(const Json::Reader& jsonReader);
		void Remove(const Json::Reader& jsonReader);
		void GetServiceInfo(Json::Writer& jsonWriter);
	 private:
		int mAreaId;
		bool mIsStartComplete;
		std::string mNodeName;
		class RedisComponent* mRedisComponent;
		class HttpClientComponent* mHttpComponent;
	};
}