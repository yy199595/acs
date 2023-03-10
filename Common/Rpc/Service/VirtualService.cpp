//
// Created by yjz on 2022/6/5.
//

#include "VirtualService.h"
#include"XCode/XCode.h"
namespace Sentry
{
	int VirtualService::Invoke(const std::string &name, std::shared_ptr<Rpc::Packet> message)
	{
		return XCode::CallServiceNotFound;
	}
}
