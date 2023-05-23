//
// Created by zmhy0073 on 2022/10/8.
//

#include"Node.h"
#include"Entity/Actor/App.h"
#include"Util/String/StringHelper.h"
#include"Server/Config/CodeConfig.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Server/Component/TextConfigComponent.h"

namespace Tendo
{
    bool Node::OnInit()
    {
		BIND_COMMON_RPC_METHOD(Node::Ping);
		BIND_COMMON_RPC_METHOD(Node::Join);
		BIND_COMMON_RPC_METHOD(Node::Exit);
		BIND_COMMON_RPC_METHOD(Node::Stop);
		BIND_COMMON_RPC_METHOD(Node::Hotfix);
		BIND_COMMON_RPC_METHOD(Node::RunInfo);
		BIND_COMMON_RPC_METHOD(Node::LoadConfig);
		return true;
    }

	int Node::Ping(const Msg::Packet& packet)
    {
        CONSOLE_LOG_FATAL("[" << packet.From() << "] ping server");
        return XCode::Successful;
    }

    int Node::Join(const s2s::server::info &request)
	{
		int id = request.server_id();
		const std::string & server = request.server_name();
		if (!ClusterConfig::Inst()->GetConfig(server))
		{
			LOG_ERROR("not find cluster config : " << server);
			return XCode::Failure;
		}
		if(this->mApp->ActorMgr()->GetActor(id) == nullptr)
		{
			std::shared_ptr<Server> actor = std::make_shared<Server>(id, server);
			{
				auto iter = request.listens().begin();
				for(; iter != request.listens().end(); iter++)
				{
					const std::string & name = iter->first;
					const std::string & address = iter->second;
					actor->AddListen(name, address);
				}
			}
			this->mApp->ActorMgr()->AddServer(actor);
		}
		std::vector<IServerChange *> components;
		this->mApp->GetComponents(components);
		for(IServerChange * listen : components)
		{
			listen->OnJoin(id);
		}
		return XCode::Successful;
	}

    int Node::Exit(const s2s::server::info &request)
	{
		int id = request.server_id();
		this->mApp->ActorMgr()->DelActor(id);
		std::vector<IServerChange*> components;
		this->mApp->GetComponents(components);
		for (IServerChange* listen: components)
		{
			listen->OnExit(id);
		}
		return XCode::Successful;
	}

    int Node::Stop()
    {
        std::vector<RpcService *> components;
        if(this->mApp->GetComponents(components))
        {
            for (RpcService *component: components)
            {
                // TODO
            }
        }
		const ServerConfig * config = ServerConfig::Inst();
		s2s::server::info request;
		{
			request.set_server_name(config->Name());
			request.set_server_id(config->ServerId());
		}
		const std::string func("Registry.UnRegister");
		int code = this->mApp->Call(func, request);
		this->mApp->GetCoroutine()->Start(&App::Stop, this->mApp, 0);
		const std::string & desc = CodeConfig::Inst()->GetDesc(code);
		CONSOLE_LOG_INFO("unregister " << config->Name()  << " code = " << desc);
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
}