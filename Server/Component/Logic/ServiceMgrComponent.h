#pragma once
#include"XCode/XCode.h"
#include"Component/Scene/NetEventComponent.h"
using namespace com;
namespace Sentry
{

	class TaskComponent;
	class ServiceRpcComponent;
	class ServiceMgrComponent : public NetEventComponent, public IComplete, public IServiceChange
	{
	 public:
		ServiceMgrComponent() = default;
		~ServiceMgrComponent() override = default;
	 public:
		bool LateAwake() final;
		void OnDestory() final;
		void OnComplete() final;
	 private:
		bool RefreshService();
		void OnAddService(Component *component) final;
		void OnDelService(Component *component) final;
	 private:
		bool OnServiceAdd(const Json::Reader & json);
		bool OnServiceDel(const Json::Reader & json);
		bool OnNodeRegister(const Json::Reader & json);
		void RemoveAddress(const std::string & address);
		bool QueryNodeInfo(const std::string & address);
		bool OnRegisterEvent(NetEventRegistry &eventRegister) final;
	 private:
		int mAreaId;
		std::string mNodeName;
		std::string mRpcAddress;
		std::set<std::string> mAllAddress;
		std::vector<std::string> mServices;
		std::vector<std::string> mJsonMessages;
		class MainRedisComponent * mRedisComponent;
	};
}