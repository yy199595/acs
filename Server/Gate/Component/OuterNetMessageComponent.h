
//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_PROTOPROXYCOMPONENT_H
#define GAMEKEEPER_PROTOPROXYCOMPONENT_H
#include"Client/Message.h"
#include"Component/Component.h"
#include"Async/RpcTaskSource.h"

namespace Sentry
{
	class OuterNetMessageComponent final : public Component
	{
	 public:
		OuterNetMessageComponent();
    public:
		int OnMessage(long long userId, std::shared_ptr<Rpc::Packet> message);
	public:
		int OnLogin(long long userId);
		int OnLogout(long long userId);
    private:
		bool LateAwake() final;
		int OnRequest(long long userId, std::shared_ptr<Rpc::Packet> message);
	private:
		class NodeMgrComponent * mNodeComponent;
        std::unordered_map<std::string, long long> mTokens;
        class InnerNetMessageComponent * mInnerMessageComponent;
        std::unordered_map<std::string, long long> mUserAddressMap;
        std::unordered_map<long long, std::string> mClientAddressMap;
    };
}


#endif //GAMEKEEPER_PROTOPROXYCOMPONENT_H
