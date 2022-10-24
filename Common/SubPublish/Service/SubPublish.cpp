//
// Created by yjz on 2022/10/24.
//

#include "SubPublish.h"

namespace Sentry
{
	bool SubPublish::OnStart()
	{
		BIND_COMMON_RPC_METHOD(SubPublish::Sub);
		BIND_COMMON_RPC_METHOD(SubPublish::UnSub);
		BIND_COMMON_RPC_METHOD(SubPublish::Publish);
		return true;
	}

	bool SubPublish::OnClose()
	{
		return true;
	}

	XCode SubPublish::Sub(const Rpc::Head& head, const s2s::forward::sub& request)
	{
		return XCode::Successful;
	}

	XCode SubPublish::UnSub(const Rpc::Head& head, const s2s::forward::unsub& request)
	{
		return XCode::Successful;
	}

	XCode SubPublish::Publish(const Rpc::Head& head, const s2s::forward::unsub& request)
	{
		return XCode::Successful;
	}
}