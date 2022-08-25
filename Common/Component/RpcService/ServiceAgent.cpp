//
// Created by yjz on 2022/6/5.
//

#include "ServiceAgent.h"
namespace Sentry
{
	XCode ServiceAgent::Invoke(const string& name,
		std::shared_ptr<com::rpc::request> request, std::shared_ptr<com::rpc::response> response)
	{
		return XCode::CallServiceNotFound;
	}
}
