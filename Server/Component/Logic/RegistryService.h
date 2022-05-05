#pragma once
#include"XCode/XCode.h"
#include"Protocol/sub.pb.h"
#include"Component/Service/RedisSubService.h"
using namespace com;
namespace Sentry
{
	class LocalServiceComponent;

	class RpcComponent;

	class TaskComponent;

	class RegistryService : public RedisSubService, public IComplete, public IServiceChange
	{
	 public:
		RegistryService() = default;
		~RegistryService() override = default;

	 public:
		bool LateAwake() final;
		void OnDestory() final;
		void OnComplete() final;
	 protected:
		void OnAddService(Component *component) final;
		void OnDelService(Component *component) final;
		bool OnInitService(SubServiceRegister &methodRegister) final;
	 private:
		XCode Add(const sub::Add::Request& request);
		XCode Del(const sub::Del::Request& request);
		XCode Push(const sub::Push::Request& request, sub::Push::Response & response);
	private:
		int mAreaId;
		std::string mNodeName;
		std::string mRpcAddress;
		std::string mHttpAddress;
		class MainRedisComponent * mRedisComponent;
	};
}