//
// Created by zmhy0073 on 2022/10/8.
//

#include"InnerService.h"
#include"Config/ClusterConfig.h"
#include"Component/InnerNetComponent.h"
#include"Component/LocationComponent.h"
#include"Component/TextConfigComponent.h"
#include"Component/InnerNetMessageComponent.h"

namespace Sentry
{
    void InnerService::Init()
    {
        this->mApp->AddComponent<InnerNetComponent>();
        this->mApp->AddComponent<InnerNetMessageComponent>();
    }

    bool InnerService::OnStart()
    {
        BIND_COMMON_RPC_METHOD(InnerService::Ping);
        BIND_COMMON_RPC_METHOD(InnerService::Join);
        BIND_COMMON_RPC_METHOD(InnerService::Exit);
        BIND_COMMON_RPC_METHOD(InnerService::Stop);
        BIND_COMMON_RPC_METHOD(InnerService::Hotfix);
		BIND_COMMON_RPC_METHOD(InnerService::RunInfo);
		BIND_COMMON_RPC_METHOD(InnerService::LoadConfig);
		this->mLocationComponent = this->GetComponent<LocationComponent>();
        return true;
    }

	XCode InnerService::Ping()
    {
        return XCode::Successful;
    }

    XCode InnerService::Join(const s2s::cluster::join &request)
    {
		for(int index = 0; index < request.list_size(); index++)
		{
			const s2s::cluster::server & server = request.list(index);
			if(!ClusterConfig::Inst()->GetConfig(server.name()))
			{
				LOG_ERROR("not find cluster config : " << server.name());
			}
			this->mLocationComponent->AddRpcServer(server.name(), server.rpc());
			this->mLocationComponent->AddHttpServer(server.name(), server.http());
		}
        return XCode::Successful;
    }

    XCode InnerService::Exit(const s2s::cluster::exit &response)
    {
        return XCode::Successful;
    }

    XCode InnerService::Stop()
    {
        std::vector<RpcService *> components;
        if(this->mApp->GetComponents(components))
        {
            for (RpcService *component: components)
            {
                if(component->IsStartService())
                {
                    component->WaitAllMessageComplete();
                    component->Close();
                }
            }
        }

		CONSOLE_LOG_INFO("shutdown server int 10s after");
		TimerComponent * timerComponent = this->mApp->GetTimerComponent();
		timerComponent->DelayCall(10 * 1000, &App::Stop, this->mApp);
        return XCode::Successful;
    }

    XCode InnerService::LoadConfig()
    {
        TextConfigComponent * textComponent = this->GetComponent<TextConfigComponent>();
        if(textComponent != nullptr)
        {
            textComponent->OnHotFix();
        }
        return XCode::Successful;
    }

	XCode InnerService::RunInfo(google::protobuf::StringValue& response)
	{
		Json::Document document;
		std::vector<Component *> components;
		this->mApp->GetComponents(components);
		document.Add("server", ServerConfig::Inst()->Name());
		for(Component * component : components)
		{
			IServerRecord * serverRecord = component->Cast<IServerRecord>();
			if(serverRecord != nullptr)
			{
				IServiceBase * serviceBase = component->Cast<IServiceBase>();
				if(serviceBase != nullptr && !serviceBase->IsStartService())
				{
					continue;
				}
				std::unique_ptr<Json::Document> document1(new Json::Document());
				{
					serverRecord->OnRecord(*document1);
					const char* key = component->GetName().c_str();
					document.Add(key, std::move(document1));
				}
			}
		}
		document.Serialize(response.mutable_value());
		return XCode::Successful;
	}

    XCode InnerService::Hotfix()
    {
        std::vector<IHotfix *> components;
		this->mApp->GetComponents(components);
        for(IHotfix * component : components)
        {
            component->OnHotFix();
        }
        return XCode::Successful;
    }
}