#pragma once
#include"Json/JsonWriter.h"
#include"Component/Service/SubService.h"
using namespace com;
namespace Sentry
{
	class RpcService;

	class RpcComponent;

	class TaskComponent;

	class ServiceMgrComponent;

	class LocalService : public SubService, public IStart, public ILocalService<RpcService>
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
		void OnAddService(RpcService *component) final;
		void OnDelService(RpcService *component) final;
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
		std::vector<IRemoteService *> mRemoteListeners;
	};
}