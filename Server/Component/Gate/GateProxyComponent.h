//
// Created by yjz on 2022/4/23.
//

#ifndef _GATEPROXYCOMPONENT_H_
#define _GATEPROXYCOMPONENT_H_
#include"Component/Component.h"

namespace Sentry
{
	class GateProxyComponent final : public Component
	{
	 public:
		GateProxyComponent() = default;
		~GateProxyComponent() = default;
	 private:
		bool LateAwake() final;
	 public:
		XCode Call(long long userId, const std::string & func);
		XCode Call(long long UserId, const std::string & func, const Message & message);
	 public:
		XCode BroadCast(const std::string & func);
		XCode BroadCast(const std::string & func, const Message & message);
	 private:
		class GateService * mGateService;
	};
}

#endif //_GATEPROXYCOMPONENT_H_
