#pragma once
#include"XCode/XCode.h"
#include"Protocol/sub.pb.h"
#include"Component/Component.h"
using namespace com;
namespace Sentry
{

	class TaskComponent;
	class RpcHandlerComponent;
	class RegistryService : public Component, public IComplete, public IServiceChange
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
	 private:
		XCode Add(const sub::Add::Request& request);
		XCode Del(const sub::Del::Request& request);
		XCode Push(const sub::Push::Request& request, sub::Push::Response & response);
	private:
		int mAreaId;
		std::string mNodeName;
		std::string mRpcAddress;
		class MainRedisComponent * mRedisComponent;
	};
}