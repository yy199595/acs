//
// Created by yjz on 2022/6/5.
//

#include "ServiceAgentComponent.h"
namespace Sentry
{
	XCode Sentry::ServiceAgentComponent::Invoke(const string& name,
		std::shared_ptr<com::Rpc::Request> request, std::shared_ptr<com::Rpc::Response> response)
	{
		return XCode::CallServiceNotFound;
	}
}
