
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
		OuterNetMessageComponent() = default;
		~OuterNetMessageComponent() final = default;
    public:
        bool GetAddress(long long id, std::string & address);
        bool CreateToken(long long userId, std::string & token);
    public:
        void OnClose(const std::string & address);
        XCode OnAuth(const std::string & address, std::shared_ptr<Rpc::Data> message);
        XCode OnRequest(const std::string & address, std::shared_ptr<Rpc::Data> message);
        XCode OnResponse(const std::string & address, std::shared_ptr<Rpc::Data> message);
    private:
		bool LateAwake() final;
	 private:
        class TaskComponent * mTaskComponent;
		class TimerComponent * mTimerComponent;
        class OuterNetComponent* mOutNetComponent;
		class LocationComponent * mLocationComponent;
        std::unordered_map<std::string, long long> mTokens;
        class InnerNetMessageComponent * mInnerMessageComponent;
        std::unordered_map<std::string, long long> mUserAddressMap;
        std::unordered_map<long long, std::string> mClientAddressMap;
    };
}


#endif //GAMEKEEPER_PROTOPROXYCOMPONENT_H
