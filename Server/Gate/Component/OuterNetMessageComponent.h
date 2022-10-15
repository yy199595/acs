
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
        XCode OnRequest(long long userId, std::shared_ptr<Rpc::Data> message);
        XCode OnResponse(const std::string & address, std::shared_ptr<Rpc::Data> message);
    private:
		bool LateAwake() final;
	 private:
		class TaskComponent * mTaskComponent;
		class TimerComponent * mTimerComponent;
        class OuterNetComponent* mOutNetComponent;
		class LocationComponent * mLocationComponent;
        class InnerNetMessageComponent * mInnerMessageComponent;
    };
}


#endif //GAMEKEEPER_PROTOPROXYCOMPONENT_H
