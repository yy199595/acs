//
// Created by yjz on 2022/6/5.
//

#include "ServiceRpcComponent.h"
namespace Sentry
{
	XCode ServiceRpcComponent::Invoke(const std::string &name, std::shared_ptr<Rpc::Packet> message)
	{
		return XCode::CallServiceNotFound;
	}
}
