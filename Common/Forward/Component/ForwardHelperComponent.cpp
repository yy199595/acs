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

    void ForwardHelperComponent::GetLocation(long long userId, std::string &address)
    {
        size_t index = userId % this->mLocations.size();
        address = this->mLocations[index];
    }
}