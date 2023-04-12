//
// Created by zmhy0073 on 2022/10/8.
//

#include"Node.h"
#include"Entity/Unit/App.h"
#include"Registry/Service/Registry.h"
#include"Server/Config/CodeConfig.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Rpc/Component/InnerNetComponent.h"
#include"Rpc/Component/NodeMgrComponent.h"
#include"Server/Component/TextConfigComponent.h"
#include"Rpc/Component/DispatchMessageComponent.h"

namespace Tendo
{
	Node::Node()
	{
		this->mNodeComponent = nullptr;
	}

	bool Node::Awake()
	{
		this->mApp->AddComponent<InnerNetComponent>();
		this->mApp->AddComponent<DispatchMessageComponent>();
		return true;
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

		std::vector<std::string> registryAddress;
		const ServerConfig * config = ServerConfig::Inst();
		RpcService* rpcService = this->mApp->GetService<Registry>();
		this->mNodeComponent = this->GetComponent<NodeMgrComponent>();
		if(!config->GetMember("registry", registryAddress))
		{
			LOG_ERROR("not find config registry address");
			return false;
		}
		for(const std::string & address : registryAddress)
		{
			const std::string & server = rpcService->GetServer();
			this->mNodeComponent->AddRpcServer(server, address);
		}
		return true;
    }

	int Node::Ping(const Rpc::Packet& packet)
    {
        CONSOLE_LOG_FATAL("[" << packet.From() << "] ping server");
        return XCode::Successful;
    }

	void Node::OnLocalComplete()
	{
		std::string address;
		const std::string func("Register");
		const ServerConfig* config = ServerConfig::Inst();
		RpcService* rpcService = this->mApp->GetService<Registry>();
		const std::string & server = rpcService->GetServer();
		while(this->mNodeComponent->GetServer(server, address))
		{
			s2s::server::info message;
			{
				message.set_name(config->Name());
				config->GetLocation("rpc", *message.mutable_rpc());
				config->GetLocation("http", *message.mutable_http());
			}
#ifdef __DEBUG__
			LOG_INFO("start register to [" << address << "]");
#endif
			rpcService->SetProto(Msg::Porto::Json);
			int code = rpcService->Call(address, func, message);
			if (code != XCode::Successful)
			{
				LOG_ERROR("register to [" << address << "] "
										  << CodeConfig::Inst()->GetDesc(code));
				this->mApp->GetTaskComponent()->Sleep(1000 * 5);
				continue;
			}
			const std::string & server = rpcService->GetServer();
			this->mNodeComponent->AddRpcServer(server, address);
			LOG_INFO("register to [" << address << "] successful");
			return;
		}
		LOG_FATAL("registry to registry failure");
	}

    int Node::Join(const s2s::server::info &request)
	{
		const std::string & rpc = request.rpc();
		const std::string & http = request.http();
		const std::string & name = request.name();
		if (!ClusterConfig::Inst()->GetConfig(name))
		{
			LOG_ERROR("not find cluster config : " << name);
			return XCode::Failure;
		}
		std::vector<IServerChange *> components;
		this->mApp->GetComponents(components);
		for(IServerChange * listen : components)
		{
			listen->OnJoin(name, rpc, http);
		}
		this->mNodeComponent->AddRpcServer(name, rpc);
		this->mNodeComponent->AddHttpServer(name, http);
		return XCode::Successful;
	}

    int Node::Exit(const com::type::string &request)
    {
        const std::string& rpc = request.str();
		std::vector<IServerChange *> components;
		this->mApp->GetComponents(components);

		for(IServerChange * listen : components)
		{
			listen->OnExit(request.str());
		}
        this->mNodeComponent->DelServer(rpc);
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
		taskComponent->Start(&App::Stop, this->mApp);
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
		RpcService* rpcService = this->mApp->GetService<Registry>();
		const std::string & server = rpcService->GetServer();
		if(this->mNodeComponent->GetServer(server, address))
		{
			com::type::string request;
			const ServerConfig* config = ServerConfig::Inst();
			config->GetLocation("rpc", *request.mutable_str());
			int code = rpcService->Call(address, "UnRegister", request);
			const std::string & desc = CodeConfig::Inst()->GetDesc(code);
			CONSOLE_LOG_INFO("unregister " << request.str() << " code = " << desc);
		}
	}
}