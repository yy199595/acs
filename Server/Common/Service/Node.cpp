//
// Created by zmhy0073 on 2022/10/8.
//

#include"Node.h"
#include"Entity/Actor/App.h"
#include"Server/Config/CodeConfig.h"
#include"Server/Component/ConfigComponent.h"

namespace Tendo
{
    bool Node::OnInit()
    {
		BIND_COMMON_RPC_METHOD(Node::Ping);
		BIND_COMMON_RPC_METHOD(Node::Join);
		BIND_COMMON_RPC_METHOD(Node::Exit);
		BIND_COMMON_RPC_METHOD(Node::Hotfix);
		BIND_COMMON_RPC_METHOD(Node::RunInfo);
		BIND_COMMON_RPC_METHOD(Node::Shutdown);
		BIND_COMMON_RPC_METHOD(Node::LoadConfig);
		return true;
    }

	int Node::Ping(const Msg::Packet& packet)
    {
        CONSOLE_LOG_FATAL("[" << packet.From() << "] ping server");
        return XCode::Successful;
    }

    int Node::Join(const com::type::json &request)
	{

		return XCode::Successful;
	}

    int Node::Exit(const com::type::int64 &request)
	{
		return XCode::Successful;
	}

    int Node::Shutdown()
    {
		this->mApp->GetCoroutine()->Start(&App::Stop, this->mApp, 0);
		return XCode::Successful;
    }

    int Node::LoadConfig()
    {
        ConfigComponent * textComponent = this->GetComponent<ConfigComponent>();
        if(textComponent != nullptr)
        {
            textComponent->OnHotFix();
        }
        return XCode::Successful;
    }

	int Node::RunInfo(com::type::json& response)
	{
		Json::Writer document;
        {
            document.BeginObject("server");
			document.Add("fps").Add((int)this->mApp->GetFps());
			document.Add("name").Add(ServerConfig::Inst()->Name());
            document.Add("cpu").Add(std::thread::hardware_concurrency());
            document.EndObject();
        }
		document.WriterStream(response.mutable_json());
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