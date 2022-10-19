//
// Created by zmhy0073 on 2022/10/18.
//
#include"Config/ClusterConfig.h"
#include"ForwardHelperComponent.h"
#include"Component/InnerNetMessageComponent.h"
namespace Sentry
{
    bool ForwardHelperComponent::LateAwake()
    {
        const ServerConfig * config = ServerConfig::Inst();
        LOG_CHECK_RET_FALSE(config->GetMember("server", "forward", this->mLocations));
        LOG_CHECK_RET_FALSE(this->mInnerComponent = this->GetComponent<InnerNetMessageComponent>());
        return true;
    }

    void ForwardHelperComponent::OnLocalComplete()
    {
        for(const std::string & address : this->mLocations)
        {
            if(!this->mInnerComponent->Ping(address))
            {
                LOG_ERROR("ping forward server [" << address << "] error");
            }
            else
            {
                CONSOLE_LOG_INFO("ping forward server [" << address << "] successful")
            }
        }
    }

    bool ForwardHelperComponent::SendData(std::shared_ptr<Rpc::Packet> message)
    {
        LOG_CHECK_RET_FALSE(!this->mLocations.empty());
        for(const std::string & address : this->mLocations)
        {
            this->mInnerComponent->Send(address, message);
        }
        return true;
    }

    bool ForwardHelperComponent::SendData(long long userId, std::shared_ptr<Rpc::Packet> message)
    {
        LOG_CHECK_RET_FALSE(!this->mLocations.empty());

        message->GetHead().Add("id", userId);
        size_t index = userId % this->mLocations.size();
        const std::string & address = this->mLocations[index];
        return this->mInnerComponent->Send(address, message);
    }

    bool ForwardHelperComponent::SendData(const std::string &target, std::shared_ptr<Rpc::Packet> message)
    {
        std::string address;
        LOG_CHECK_RET_FALSE(!this->mLocations.empty());
        LOG_CHECK_RET_FALSE(this->AllotLocation(address));

        message->GetHead().Add("to", target);
        return this->mInnerComponent->Send(address, message);
    }

    bool ForwardHelperComponent::AllotLocation(std::string &address)
    {
        return true;
    }
}