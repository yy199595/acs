#pragma once
#include"XCode/XCode.h"
#include"Component/Service/SubService.h"
using namespace com;
namespace Sentry
{
	class LocalServiceComponent;

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
		XCode Add(const Json::Reader& jsonReader);
		XCode Remove(const Json::Reader& jsonReader);
		XCode Push(const Json::Reader& jsonReader, Json::Writer & response);
	private:
		int mAreaId;
		std::string mNodeName;
	};
}