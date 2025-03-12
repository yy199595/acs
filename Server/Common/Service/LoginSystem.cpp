//
// Created by zmhy0073 on 2022/10/13.
//

#include"LoginSystem.h"
#include"Entity/Actor/App.h"
#include"Core/Event/IEvent.h"
#include"Lua/Module/LuaModule.h"
#include "Common/Entity/Player.h"

namespace acs
{
	LoginSystem::LoginSystem()
	{
		this->mActor = nullptr;
	}

	bool LoginSystem::OnInit()
	{
		BIND_SERVER_RPC_METHOD(LoginSystem::Login);
		BIND_SERVER_RPC_METHOD(LoginSystem::Logout);
		this->mApp->GetComponents(this->mLoginComponents);
		this->mActor = this->GetComponent<ActorComponent>();
		return true;
	}

    int LoginSystem::Login(long long id, const s2s::login::request & request)
	{
		int gateId = (int)id;
		int sockId = request.client_id();
		long long playerId = request.user_id();
		if(this->mActor->GetActor(playerId) == nullptr)
		{
			std::unique_ptr<Player> player = std::make_unique<Player>(playerId, gateId, sockId);
			{
				for (int index = 0; index < request.list_size(); index++)
				{
					const auto& serverInfo = request.list(index);
					player->AddServer(serverInfo.name(), serverInfo.id());
				}
			}
		}
		Lua::LuaModule * luaModule = this->GetLuaModule();
		if(luaModule != nullptr && luaModule->HasFunction("OnLogin"))
		{
			luaModule->Await("OnLogin", playerId);
		}
		help::PlayerLoginEvent::Trigger(playerId, sockId);
		return XCode::Ok;
	}

    int LoginSystem::Logout(long long id, const s2s::logout::request& request)
    {
		long long playerId = request.user_id();
		Player * player = this->mActor->GetActor<Player>(playerId);
		if(player == nullptr)
		{
			return XCode::NotFindUser;
		}
		for(ILogin * loginComponent : this->mLoginComponents)
		{
			loginComponent->OnLogout(playerId);
		}
		Lua::LuaModule * luaModule = this->GetLuaModule();
		if(luaModule != nullptr && luaModule->HasFunction("OnLogout"))
		{
			luaModule->Await("OnLogout", playerId);
		}
		int sockId = player->GetClientID();
		this->mActor->DelActor(playerId);
		help::PlayerLogoutEvent::Trigger(playerId, sockId);
		return XCode::Ok;
    }
}