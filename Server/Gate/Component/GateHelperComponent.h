//
// Created by yjz on 2022/4/23.
//

#ifndef _GATEAGENTCOMPONENT_H_
#define _GATEAGENTCOMPONENT_H_
#include"Component/Component.h"
#include"google/protobuf/message.h"
using namespace google::protobuf;
namespace Sentry
{
	class GateHelperComponent final : public Component, public ILuaRegister
	{
	 public:
		GateHelperComponent() = default;
		~GateHelperComponent() = default;
    public:
		int Call(long long userId, const std::string & func);
        int Call(long long UserId, const std::string & func, const Message & message);
	 public:
		int BroadCast(const std::string & func);
		int BroadCast(const std::string & func, const Message & message);
	 private:		
		int LuaBroadCast(const char * func, std::shared_ptr<Message> message);
		int LuaCall(long long userId, const std::string func, std::shared_ptr<Message> message);
	 protected:
		bool LateAwake() final;
		void OnLuaRegister(Lua::ClassProxyHelper & luaRegister) final;
	 private:
		class RpcService * mGate;
		class NodeMgrComponent* mNodeComponent{};
        class InnerNetComponent * mInnerComponent{};
	};
}

#endif //_GATEAGENTCOMPONENT_H_
