#pragma once
#include"XCode/XCode.h"
#include"Protocol/sub.pb.h"
#include"Component/Component.h"
using namespace com;
namespace Sentry
{

	class TaskComponent;
	class RpcHandlerComponent;
	class ServiceMgrComponent : public Component, public IComplete, public IServiceChange
	{
	 public:
		ServiceMgrComponent() = default;
		~ServiceMgrComponent() override = default;
	 public:
		bool LateAwake() final;
		void OnDestory() final;
		void OnComplete() final;
	 protected:
		void OnAddService(Component *component) final;
		void OnDelService(Component *component) final;
	private:
		int mAreaId;
		std::string mNodeName;
		std::string mRpcAddress;
		class MainRedisComponent * mRedisComponent;
	};
}