//
// Created by zmhy0073 on 2022/10/8.
//

#include"Node.h"
#include"Config/ClusterConfig.h"
#include"Component/InnerNetComponent.h"
#include"Component/LocationComponent.h"
#include"Component/TextConfigComponent.h"
#include"Component/InnerNetMessageComponent.h"

namespace Sentry
{
    void Node::Init()
    {
        this->mApp->AddComponent<InnerNetComponent>();
        this->mApp->AddComponent<InnerNetMessageComponent>();
    }

    bool Node::OnStart()
    {
        BIND_COMMON_RPC_METHOD(Node::Ping);
        BIND_COMMON_RPC_METHOD(Node::Join);
        BIND_COMMON_RPC_METHOD(Node::Exit);
        BIND_COMMON_RPC_METHOD(Node::Stop);
        BIND_COMMON_RPC_METHOD(Node::Hotfix);
		BIND_COMMON_RPC_METHOD(Node::RunInfo);
		BIND_COMMON_RPC_METHOD(Node::LoadConfig);
		this->mLocationComponent = this->GetComponent<LocationComponent>();
        return true;
    }

	XCode Node::Ping()
    {
        return XCode::Successful;
    }

    XCode Node::Join(const s2s::cluster::server &request)
	{
		const std::string & rpc = request.rpc();
		const std::string & http = request.http();
		const std::string & name = request.name();
		if (!ClusterConfig::Inst()->GetConfig(name))
		{
			LOG_ERROR("not find cluster config : " << name);
			return XCode::Failure;
		}
		this->mLocationComponent->AddRpcServer(name, rpc);
		this->mLocationComponent->AddHttpServer(name, http);
		return XCode::Successful;
	}

    XCode Node::Exit(const s2s::cluster::server &response)
    {
        return XCode::Successful;
    }

    XCode Node::Stop()
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

    XCode Node::LoadConfig()
    {
        TextConfigComponent * textComponent = this->GetComponent<TextConfigComponent>();
        if(textComponent != nullptr)
        {
            textComponent->OnHotFix();
        }
        return XCode::Successful;
    }

	XCode Node::RunInfo(google::protobuf::StringValue& response)
	{
		Json::Writer document;
        {
            document.BeginObject("server");
            document.Add("name").Add(ServerConfig::Inst()->Name());
            document.Add("fps").Add((int)this->mApp->GetFps());
            document.Add("cpu").Add(std::thread::hardware_concurrency());
            document.Add(Json::End::EndObject);
        }
        std::vector<Component *> components;
        this->mApp->GetComponents(components);
		for(Component * component : components)
		{
            const char* key = component->GetName().c_str();
			IServerRecord * serverRecord = component->Cast<IServerRecord>();
			if(serverRecord != nullptr)
			{
				IServiceBase * serviceBase = component->Cast<IServiceBase>();
				if(serviceBase != nullptr && !serviceBase->IsStartService())
				{
					continue;
				}
                document.BeginObject(key);
                serverRecord->OnRecord(document);
                document.Add(Json::End::EndObject);				
			}
		}
		document.WriterStream(response.mutable_value());
		return XCode::Successful;
	}

    XCode Node::Hotfix()
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