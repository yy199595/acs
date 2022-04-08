#pragma once
#include"Json/JsonWriter.h"
#include"Component/Service/SubService.h"
using namespace com;
namespace Sentry
{
	class LocalServerRpc;

	class RpcComponent;

	class TaskComponent;

	class LocalService : public SubService, public IComplete, public IServiceChange
	{
	 public:
		LocalService() = default;
		~LocalService() override = default;

	 public:
		bool Awake() final;
		bool LateAwake() final;
		void OnDestory() final;
		void OnComplete() final;
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