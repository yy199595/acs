//
// Created by yjz on 2022/4/23.
//

#ifndef _GATEAGENTCOMPONENT_H_
#define _GATEAGENTCOMPONENT_H_
#include"Component/Scene/NetEventComponent.h"
namespace Sentry
{
	class GateAgentComponent final : public Component, public ILuaRegister
	{
	 public:
		GateAgentComponent() = default;
		~GateAgentComponent() = default;
	 public:
		XCode Call(long long userId, const std::string & func);
		XCode Call(long long UserId, const std::string & func, const Message & message);
	 public:
		XCode BroadCast(const std::string & func);
		XCode BroadCast(const std::string & func, const Message & message);
	 private:
		XCode LuaBroadCast(const char * func, std::shared_ptr<Message> message);
		XCode LuaCall(long long userId, const std::string func, std::shared_ptr<Message> message);
	 protected:
		bool LateAwake() final;
		void OnLuaRegister(Lua::ClassProxyHelper & luaRegister) final;
	 private:
		class GateService * mGateService;
	};
}

#endif //_GATEAGENTCOMPONENT_H_
