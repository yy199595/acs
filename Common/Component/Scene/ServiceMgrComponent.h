#pragma once

#include"Component/Component.h"

namespace Sentry
{
	class ServiceProxy;
	class ServiceMgrComponent : public Component, public IRemoteService
	{
	 public:
		ServiceMgrComponent() = default;
		~ServiceMgrComponent() override = default;
	 protected:
		bool Awake() final;
		bool LateAwake() final;
		void OnServiceExit(const std::string &address) final;
		void OnServiceJoin(const std::string &name, const std::string &address) final;
	 public:
		const std::string QueryAddress(const std::string & name);
		std::shared_ptr<ServiceProxy> GetServiceProxy(const std::string& name);
	 private:
		std::unordered_map<std::string, std::shared_ptr<ServiceProxy>> mServiceEntityMap;
	};
}