//
// Created by zmhy0073 on 2022/10/14.
//

#ifndef APP_FORWARDCOMPONENT_H
#define APP_FORWARDCOMPONENT_H
#include"Client/Message.h"
#include"Client/InnerNetClient.h"
#include"Component/Component.h"

namespace Sentry
{
    class TranComponent : public Component
    {
    public:
        TranComponent() = default;
        ~TranComponent() = default;
    private:		
        bool LateAwake() final;
    public:
        XCode OnRequest(std::shared_ptr<Rpc::Packet> message);
		XCode OnResponse(std::shared_ptr<Rpc::Packet> message);
    private:
        XCode Forward(long long userId, std::shared_ptr<Rpc::Packet> message);
        XCode Forward(const std::string & adress, std::shared_ptr<Rpc::Packet> message);
    private:
        class InnerNetComponent* mInnerComponent;
        class LocationComponent * mLocationComponent;
    };
}

#endif //APP_FORWARDCOMPONENT_H
