//
// Created by yjz on 2022/6/5.
//

#include "ServiceWrapper.h"
namespace Sentry
{
	XCode ServiceWrapper::Invoke(const std::string &name, std::shared_ptr<Rpc::Data> message)
	{
		return XCode::CallServiceNotFound;
	}
}
