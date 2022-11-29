//
// Created by zmhy0073 on 2022/10/18.
//
#include"Service/RpcService.h"
#include"Config/ClusterConfig.h"
#include"Unit/LocationUnit.h"
#include"ForwardHelperComponent.h"
#include"Service/LocationService.h"
#include"Component/LocationComponent.h"
#include"Component/InnerNetMessageComponent.h"
namespace Sentry
{
    bool ForwardHelperComponent::LateAwake()
    {
        const ServerConfig * config = ServerConfig::Inst();
        this->mBindName = ComponentFactory::GetName<LocationService>();
        LOG_CHECK_RET_FALSE(config->GetMember("forward", this->mLocations));
        LOG_CHECK_RET_FALSE(this->mInnerComponent = this->GetComponent<InnerNetMessageComponent>());
        return this->mLocations.size() > 0;
    }

    void ForwardHelperComponent::OnLocalComplete()
    {
		TaskComponent * taskComponent = this->GetComponent<TaskComponent>();
        for (const std::string &address: this->mLocations)
        {
			int count = 0;
			while(!this->mInnerComponent->Ping(address))
			{
				count++;
				taskComponent->Sleep(2000);
				LOG_ERROR("try connect forward server count = " << count);
			}
            CONSOLE_LOG_INFO("ping forward server [" << address << "] successful")
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

    bool ForwardHelperComponent::OnAllot(LocationUnit *locationUnit)
    {
        std::vector<std::string> services;
        if(!locationUnit->Get(services))
        {
            return false;
        }
        std::string address;
        s2s::location::add message;
        for(const std::string & service : services)
        {
            LOG_CHECK_RET_FALSE(locationUnit->Get(service, address))
            message.mutable_services()->insert({service, address});
        }
        message.set_user_id(locationUnit->GetUnitId());
        this->GetLocation(locationUnit->GetUnitId(), address);
        RpcService * locationService = this->mApp->GetService(this->mBindName);
        if(locationService == nullptr)
        {
            LOG_ERROR("not find rpc service : " << this->mBindName);
            return false;
        }
        return locationService->Send(address, "Add", message) == XCode::Successful;
    }

    bool ForwardHelperComponent::OnAllot(const std::string &service, LocationUnit *locationUnit)
    {
        std::string address;
        if (!locationUnit->Get(service, address))
        {
            return false;
        }
        s2s::location::add message;
        message.mutable_services()->insert(
            {
                service, address
            });
        this->GetLocation(locationUnit->GetUnitId(), address);
        RpcService *locationService = this->mApp->GetService(this->mBindName);
        if(locationService == nullptr)
        {
            LOG_ERROR("not find rpc service : " << this->mBindName);
            return false;
        }
        return locationService->Send(address, message) == XCode::Successful;
    }
}