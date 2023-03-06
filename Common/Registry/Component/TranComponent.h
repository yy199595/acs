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
        int OnRequest(std::shared_ptr<Rpc::Packet> message);
		int OnResponse(std::shared_ptr<Rpc::Packet> message);
    private:
        int Forward(long long userId, std::shared_ptr<Rpc::Packet> message);
        int Forward(const std::string & address, std::shared_ptr<Rpc::Packet> message);
    private:
        class InnerNetComponent* mInnerComponent;
        class NodeMgrComponent * mLocationComponent;
    };
}

#endif //APP_FORWARDCOMPONENT_H
