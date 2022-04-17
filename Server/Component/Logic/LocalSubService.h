#pragma once
#include"Json/JsonWriter.h"
#include"Component/Service/SubService.h"
using namespace com;
namespace Sentry
{
	class LocalServerRpc;

	class RpcComponent;

	class TaskComponent;

	class LocalSubService : public SubService, public IComplete, public IServiceChange
	{
	 public:
		LocalSubService() = default;
		~LocalSubService() override = default;

	 public:
		bool LateAwake() final;
		void OnDestory() final;
		void OnComplete() final;
	 protected:
		void OnAddService(Component *component) final;
		void OnDelService(Component *component) final;
		bool OnInitService(SubServiceRegister &methodRegister) final;
	 private:
		void Add(const Json::Reader& jsonReader);
		void Push(const Json::Reader& jsonReader);
		void Remove(const Json::Reader& jsonReader);
	 private:
		int mAreaId;
		std::string mNodeName;
		std::string mRpcAddress;
		std::string mHttpService;
	};
}