//
// Created by yjz on 2022/4/23.
//

#ifndef _GATEPROXYCOMPONENT_H_
#define _GATEPROXYCOMPONENT_H_
#include"Component/Scene/NetEventComponent.h"
namespace Sentry
{
	class GateProxyComponent final : public Component, public ILuaRegister
	{
	 public:
		GateProxyComponent() = default;
		~GateProxyComponent() = default;
	 public:
		XCode Call(long long userId, const std::string & func);
		XCode Call(long long UserId, const std::string & func, const Message & message);
		XCode LuaCall(long long userId, const std::string func, const std::string pb, const std::string & json);
	 public:
		XCode BroadCast(const std::string & func);
		XCode BroadCast(const std::string & func, const Message & message);
		XCode LuaBroadCast(const std::string func, const std::string pb, const std::string & json);
	 protected:
		bool LateAwake() final;
		void OnLuaRegister(Lua::ClassProxyHelper & luaRegister) final;
	 private:
		class GateService * mGateService;
	};
}

#endif //_GATEPROXYCOMPONENT_H_
