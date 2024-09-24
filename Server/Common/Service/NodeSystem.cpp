//
// Created by zmhy0073 on 2022/10/8.
//

#include"NodeSystem.h"
#include"Entity/Actor/App.h"
#include"Core/System/System.h"
#include"Yyjson/Document/Document.h"
#include"Lua/Component/LuaComponent.h"
#include"Log/Component/LoggerComponent.h"
#include"Server/Component/ConfigComponent.h"
namespace acs
{
	NodeSystem::NodeSystem() : mActComponent(nullptr)
	{

	}

    bool NodeSystem::OnInit()
    {
		BIND_SERVER_RPC_METHOD(NodeSystem::Add);
		BIND_SERVER_RPC_METHOD(NodeSystem::Del);
		BIND_SERVER_RPC_METHOD(NodeSystem::Ping);
		BIND_SERVER_RPC_METHOD(NodeSystem::Find);
		BIND_SERVER_RPC_METHOD(NodeSystem::Hotfix);
		BIND_SERVER_RPC_METHOD(NodeSystem::RunInfo);
		BIND_SERVER_RPC_METHOD(NodeSystem::Shutdown);
		BIND_SERVER_RPC_METHOD(NodeSystem::LoadConfig);
		this->mActComponent = this->GetComponent<ActorComponent>();
		return true;
    }

	int NodeSystem::Ping(long long id)
    {
        return XCode::Ok;
    }

    int NodeSystem::Add(const com::type::json &request)
	{
		json::r::Document document;
		if(!document.Decode(request.json()))
		{
			return XCode::ParseJsonFailure;
		}
		int id = 0;
		document.Get("id", id);
		if(this->mActComponent->HasServer(id))
		{
			this->mActComponent->UpdateServer(document);
			return XCode::Ok;
		}
		Server * newServer = this->mActComponent->MakeServer(document);
		LOG_ERROR_RETURN_CODE(newServer != nullptr, XCode::CallArgsError);
		LOG_DEBUG("add new server : {}", newServer->Name());
		this->mActComponent->AddServer(std::move(newServer));
		return XCode::Ok;
	}

    int NodeSystem::Del(const com::type::int64 &request)
	{
		long long actorId = request.value();
		this->mActComponent->DelActor(actorId);
		return XCode::Ok;
	}

    int NodeSystem::Shutdown()
	{
		this->mApp->Stop();
		return XCode::Ok;
	}

	int NodeSystem::Find(com::type::json& response)
	{
		this->mApp->EncodeToJson(response.mutable_json());
		return XCode::Ok;
	}

    int NodeSystem::LoadConfig()
    {
        ConfigComponent * textComponent = this->GetComponent<ConfigComponent>();
        if(textComponent != nullptr)
        {
            textComponent->OnHotFix();
        }
        return XCode::Ok;
    }

	int NodeSystem::RunInfo(com::type::json& response)
	{
		os::SystemInfo systemInfo;
		os::System::GetSystemInfo(systemInfo);
		json::w::Document document;
        {
			document.Add("name", this->mApp->Name());
			document.Add("id", this->mApp->GetSrvId());
			document.Add("fps", this->mApp->GetFps());
			document.Add("time", this->mApp->StartTime());

			document.Add("cpu", systemInfo.cpu);
			document.Add("use_memory", systemInfo.use_memory);
			document.Add("max_memory", systemInfo.max_memory);

			std::vector<IServerRecord *> records;
			this->mApp->GetComponents(records);
			for(IServerRecord * record : records)
			{
				record->OnRecord(document);
			}
		}
		document.Encode(response.mutable_json());
		return XCode::Ok;
	}

    int NodeSystem::Hotfix()
    {
        std::vector<IHotfix *> components;
		this->mApp->GetComponents(components);
        for(IHotfix * component : components)
        {
            component->OnHotFix();
        }
        return XCode::Ok;
    }
}