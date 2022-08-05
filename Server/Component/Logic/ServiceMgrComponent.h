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
        void Update();
        bool RegisterService();
		void OnAddService(Component *component) final;
		void OnDelService(Component *component) final;
        bool RefreshService(const std::string & address);
    private:
		bool OnServiceAdd(const Json::Reader & json);
		bool OnServiceDel(const Json::Reader & json);
		bool OnNodeRegister(const Json::Reader & json);
		void RemoveAddress(const std::string & address);
		bool OnRegisterEvent(NetEventRegistry &eventRegister) final;
        std::shared_ptr<Json::Writer> GetServiceJson(bool broacast);
	 private:
		int mAreaId;
		std::string mNodeName;
		std::string mRpcAddress;
		std::vector<std::string> mServices;
        class TaskComponent * mTaskComponent;
        std::vector<std::string> mJsonMessages;
		class MainRedisComponent * mRedisComponent;
        std::unordered_map<std::string, std::set<std::string>> mAddressInfos;
	};
}