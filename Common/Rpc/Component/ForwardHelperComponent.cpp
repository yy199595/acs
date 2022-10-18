//
// Created by zmhy0073 on 2022/10/18.
//
#include"Config/ClusterConfig.h"
#include"ForwardHelperComponent.h"
#include"Component/InnerNetComponent.h"
namespace Sentry
{
    bool ForwardHelperComponent::LateAwake()
    {
        this->mInnerComponent = this->GetComponent<InnerNetComponent>();
        const NodeConfig *nodeConfig = ClusterConfig::Inst()->GetConfig("ForwardServer");
        if (nodeConfig == nullptr)
        {
            LOG_ERROR("not config cluster ForwardServer");
            return false;
        }
        return nodeConfig->GetLocations(this->mLocations);
    }

    bool ForwardHelperComponent::SendData(long long userId, std::shared_ptr<Rpc::Data> message)
    {
        LOG_CHECK_RET_FALSE(!this->mLocations.empty());
        LOG_CHECK_RET_FALSE(message->GetType() == (int)Tcp::Type::Request);

        message->GetHead().Add("id", userId);
        size_t index = userId % this->mLocations.size();
        const std::string & address = this->mLocations[index];
        return this->mInnerComponent->Send(address, message);
    }

    bool ForwardHelperComponent::SendData(const std::string &target, std::shared_ptr<Rpc::Data> message)
    {
        std::string address;
        LOG_CHECK_RET_FALSE(!this->mLocations.empty());
        LOG_CHECK_RET_FALSE(this->AllotLocation(address));
        LOG_CHECK_RET_FALSE(message->GetType() == (int)Tcp::Type::Request);

        message->GetHead().Add("to", target);
        return this->mInnerComponent->Send(address, message);
    }

    bool ForwardHelperComponent::AllotLocation(std::string &address)
    {
        return true;
    }
}