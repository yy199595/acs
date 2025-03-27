//
// Created by zmhy0073 on 2022/10/8.
//

#include "NodeSystem.h"
#include "Entity/Actor/App.h"
#include "Core/System/System.h"
#include "Util/Tools/TimeHelper.h"
#include "Yyjson/Document/Document.h"
#include "Lua/Component/LuaComponent.h"
#include "Server/Component/ConfigComponent.h"
#include "Core/Event/IEvent.h"

#ifdef __SHARE_PTR_COUNTER__

#include "Rpc/Client/InnerTcpClient.h"
#include "Rpc/Client/OuterTcpClient.h"
#include "Http/Client/RequestClient.h"
#include "Http/Client/SessionClient.h"
#include "Async/Lua/LuaWaitTaskSource.h"
#include "Redis/Client/RedisDefine.h"
#include "Http/Common/HttpRequest.h"
#include "Http/Common/HttpResponse.h"
#include "Mongo/Client/MongoProto.h"

#endif

namespace acs
{
	NodeSystem::NodeSystem() : mActor(nullptr)
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
		this->mActor = this->GetComponent<ActorComponent>();
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
		std::string name;
		LOG_ERROR_CHECK_ARGS(document.Get("id", id))
		LOG_ERROR_CHECK_ARGS(document.Get("name", name))
		std::unique_ptr<Server> server = std::make_unique<Server>(id, name);
		{
			this->mActor->DelActor(id);
			if(!server->Decode(document))
			{
				return false;
			}
			help::AddServerEvent::Trigger(name, id);
			this->mActor->AddActor(std::move(server), true);
		}
		return XCode::Ok;
	}

    int NodeSystem::Del(const com::type::int32 &request)
	{
		int actorId = request.value();
		this->mActor->DelActor(actorId);
		help::DelServerEvent::Trigger(actorId);
		return XCode::Ok;
	}

    int NodeSystem::Shutdown()
	{
		this->mApp->Stop();
		return XCode::Ok;
	}

	int NodeSystem::Find(com::type::json& response)
	{
		json::w::Document document;
		this->mApp->Encode(document);
		document.Encode(response.mutable_json());
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
			long long start = this->mApp->StartTime() / 1000;
			document.Add("start_time", help::Time::GetDateString(start));

			document.Add("cpu", fmt::format("{:.3f}%", systemInfo.cpu * 100));
			constexpr double MB = 1024 * 1024.0f;

			document.Add("use_memory", fmt::format("{:.3f}MB", systemInfo.use_memory / MB));
			document.Add("max_memory", fmt::format("{:.3f}GB", systemInfo.max_memory / (MB * 1024)));

#ifdef __SHARE_PTR_COUNTER__
			size_t count1 = rpc::Message::GetObjectCount();
			size_t count2 = rpc::InnerTcpClient::GetObjectCount();
			size_t count3 = rpc::OuterTcpClient::GetObjectCount();
			size_t count10 = http::Request::GetObjectCount();
			size_t count11 = http::Response::GetObjectCount();
			size_t count4 = http::SessionClient::GetObjectCount();
			size_t count5 = http::RequestClient::GetObjectCount();
			size_t count6 = this->mApp->ActorMgr()->GetPlayerCount();
			size_t count7 = acs::LuaWaitTaskSource::GetObjectCount();

			size_t count8 = tcp::Socket::GetObjectCount();
			size_t count9 = TaskContext::GetObjectCount();

			size_t count12 = redis::Request::GetObjectCount();
			size_t count13 = redis::Response::GetObjectCount();
			size_t count14 = RpcTaskSource::GetObjectCount();
			size_t count15 = LuaRpcTaskSource::GetObjectCount();

			size_t count16 = StaticMethod::GetObjectCount();
			size_t count17 = mongo::Request::GetObjectCount();
			size_t count18 = mongo::Response::GetObjectCount();
			std::shared_ptr<json::w::Value> jsonValue = document.AddObject("memory");
			{
				jsonValue->Add("rpc_message", count1);
				jsonValue->Add("inner_client", count2);
				jsonValue->Add("outer_client", count3);
				jsonValue->Add("http_client", count5);
				jsonValue->Add("http_session", count4);
				jsonValue->Add("http_request", count10);
				jsonValue->Add("http_response", count11);
				jsonValue->Add("player_count", count6);
				jsonValue->Add("lua_wait_task", count7);
				jsonValue->Add("rpc_task", count14);
				jsonValue->Add("lua_rpc_task", count15);
				jsonValue->Add("socket_count", count8);
				jsonValue->Add("coroutine_count", count9);
				jsonValue->Add("redis_request", count12);
				jsonValue->Add("redis_response", count13);
				jsonValue->Add("method", count16);
				jsonValue->Add("mongo_request", count17);
				jsonValue->Add("mongo_response", count18);
			}
#endif
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