//
// Created by yjz on 2022/4/23.
//

#ifndef _GATEAGENTCOMPONENT_H_
#define _GATEAGENTCOMPONENT_H_
#include"Component/Component.h"
namespace Sentry
{
	class GateHelperComponent final : public Component, public ILuaRegister
	{
	 public:
		GateHelperComponent() = default;
		~GateHelperComponent() = default;
    public:
		XCode Call(long long userId, const std::string & func);
        XCode Call(long long UserId, const std::string & func, const Message & message);
	 public:
		XCode BroadCast(const std::string & func);
		XCode BroadCast(const std::string & func, const Message & message);
	 private:
		bool GetLocation(long long userId, std::string & address);
		XCode LuaBroadCast(const char * func, std::shared_ptr<Message> message);
		XCode LuaCall(long long userId, const std::string func, std::shared_ptr<Message> message);
	 protected:
		bool LateAwake() final;
		void OnLuaRegister(Lua::ClassProxyHelper & luaRegister) final;
	 private:
		 std::string mGateServerName;
        class InnerNetComponent * mInnerComponent;
		class LocationComponent * mLocationComponent;
	};
}

#endif //_GATEAGENTCOMPONENT_H_
