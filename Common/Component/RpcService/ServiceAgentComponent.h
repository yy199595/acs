//
// Created by yjz on 2022/6/5.
//

#ifndef _SERVICEAGENTCOMPONENT_H_
#define _SERVICEAGENTCOMPONENT_H_
#include"ServiceComponent.h"
namespace Sentry
{
	class ServiceAgentComponent : public ServiceComponent
	{
	 public:
		ServiceAgentComponent() = default;
		~ServiceAgentComponent() = default;
	 private:
		bool StartNewService() final { return false;}
		bool CloseService() final { return false;}
		bool IsStartService() final { return false; }
	 private:
		XCode Invoke(const std::string& name, std::shared_ptr<com::Rpc::Request> request,
			std::shared_ptr<com::Rpc::Response> response) final;
	};
}

#endif //_SERVICEAGENTCOMPONENT_H_
