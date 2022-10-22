//
// Created by zmhy0073 on 2022/10/18.
//
#include"Service/RpcService.h"
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
        return this->mLocations.size() > 0;
    }

    void ForwardHelperComponent::OnLocalComplete()
    {
        for(const std::string & address : this->mLocations)
		{
			if (!this->mInnerComponent->Ping(address))
			{
				LOG_ERROR("ping forward server [" << address << "] error");
			}
			else
			{
				CONSOLE_LOG_INFO("ping forward server [" << address << "] successful")
			}
		}
	}

    void ForwardHelperComponent::GetLocation(std::string &address)
    {
        address = this->mLocations[0];
    }

    void ForwardHelperComponent::GetLocation(long long userId, std::string &address)
    {
        size_t index = userId % this->mLocations.size();
        address = this->mLocations[index];
    }

    bool ForwardHelperComponent::OnAllot(long long userId, const std::string &service, const std::string &address)
    {
        const std::string func("Allot");
        std::shared_ptr<Rpc::Packet> message = std::make_shared<Rpc::Packet>();
        {
            message->SetType(Tcp::Type::Request);
            message->GetHead().Add("func", func);
            message->GetHead().Add("user_id", userId);
            message->GetHead().Add(service, address);
        }
        std::string location;
        this->GetLocation(userId, location);
        return this->mInnerComponent->Send(location, message);
    }

    bool ForwardHelperComponent::OnAllot(long long userId, const std::unordered_map<std::string, std::string> &infos)
    {
        const std::string func("Allot");
        std::shared_ptr<Rpc::Packet> message = std::make_shared<Rpc::Packet>();
        {
            message->SetType(Tcp::Type::Request);
            auto iter = infos.begin();
            for (; iter != infos.end(); iter++)
            {
                const std::string & service = iter->first;
                const std::string & location = iter->second;
                message->GetHead().Add(service, location);
            }
            message->GetHead().Add("func", func);
            message->GetHead().Add("user_id", userId);
        }
        std::string address;
        this->GetLocation(userId, address);
        return this->mInnerComponent->Send(address, message);
    }
}