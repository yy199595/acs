//
// Created by zmhy0073 on 2022/10/8.
//

#include"Node.h"
#include"Entity/Unit/App.h"
#include"Util/String/StringHelper.h"
#include"Registry/Service/Registry.h"
#include"Server/Config/CodeConfig.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Rpc/Component/NodeMgrComponent.h"
#include"Server/Component/TextConfigComponent.h"

namespace Tendo
{
	Node::Node()
	{
		this->mNodeComponent = nullptr;
	}

    bool Node::OnInit()
    {
		BIND_COMMON_RPC_METHOD(Node::Ping);
		BIND_COMMON_RPC_METHOD(Node::Join);
		BIND_COMMON_RPC_METHOD(Node::Exit);
		BIND_COMMON_RPC_METHOD(Node::Stop);
		BIND_COMMON_RPC_METHOD(Node::Hotfix);
		BIND_COMMON_RPC_METHOD(Node::RunInfo);
		BIND_COMMON_RPC_METHOD(Node::LoadConfig);
		this->mNodeComponent = this->GetComponent<NodeMgrComponent>();
		return true;
    }

	int Node::Ping(const Msg::Packet& packet)
    {
        CONSOLE_LOG_FATAL("[" << packet.From() << "] ping server");
        return XCode::Successful;
    }

    int Node::Join(const s2s::server::info &request)
	{
		const std::string & server = request.name();
		if (!ClusterConfig::Inst()->GetConfig(server))
		{
			LOG_ERROR("not find cluster config : " << server);
			return XCode::Failure;
		}
		int id = request.id();
		auto iter = request.listens().begin();
		for(; iter != request.listens().end(); iter++)
		{
			const std::string & name = iter->first;
			const std::string & address = iter->second;
			this->mNodeComponent->AddServer(id, server, name, address);
		}
		std::vector<IServerChange *> components;
		this->mApp->GetComponents(components);
		for(IServerChange * listen : components)
		{
			listen->OnJoin(id);
		}
		return XCode::Successful;
	}

    int Node::Exit(const com::type::int32 &request)
    {
       	int id = request.value();
		std::vector<IServerChange *> components;
		this->mApp->GetComponents(components);
		for(IServerChange * listen : components)
		{
			listen->OnExit(id);
		}
		this->mNodeComponent->DelServer(id);
		return XCode::Successful;
    }

    int Node::Stop()
    {
        std::vector<RpcService *> components;
        if(this->mApp->GetComponents(components))
        {
            for (RpcService *component: components)
            {
                if(component != this && component->IsStartService())
                {
                    component->WaitAllMessageComplete();
                    component->Close();
                }
            }
        }
		AsyncMgrComponent * taskComponent = this->mApp->GetTaskComponent();
		taskComponent->Start(&App::Stop, this->mApp, 0);
        return XCode::Successful;
    }

    int Node::LoadConfig()
    {
        TextConfigComponent * textComponent = this->GetComponent<TextConfigComponent>();
        if(textComponent != nullptr)
        {
            textComponent->OnHotFix();
        }
        return XCode::Successful;
    }

	int Node::RunInfo(com::type::string& response)
	{
		Json::Writer document;
        {
            document.BeginObject("server");
			document.Add("fps").Add((int)this->mApp->GetFps());
			document.Add("name").Add(ServerConfig::Inst()->Name());
            document.Add("cpu").Add(std::thread::hardware_concurrency());
            document.EndObject();
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
                document.EndObject();
			}
		}
		document.WriterStream(response.mutable_str());
		return XCode::Successful;
	}

    int Node::Hotfix()
    {
        std::vector<IHotfix *> components;
		this->mApp->GetComponents(components);
        for(IHotfix * component : components)
        {
            component->OnHotFix();
        }
        return XCode::Successful;
    }

	void Node::OnClose()
	{
		std::string address;
		const ServerConfig * config = ServerConfig::Inst();
		RpcService* rpcService = this->mApp->GetService<Registry>();

		com::type::int32 request;
		request.set_value(config->GetId());
		int code = rpcService->Call(address, "UnRegister", request);
		const std::string & desc = CodeConfig::Inst()->GetDesc(code);
		CONSOLE_LOG_INFO("unregister " << config->Name()  << " code = " << desc);
	}
}