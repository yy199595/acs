//
// Created by zmhy0073 on 2022/10/13.
//

#include"LoginSystem.h"
#include"Entity/Actor/App.h"
#include"Core/Event/IEvent.h"
#include"Lua/Module/LuaModule.h"
#include"Master/Component/MasterComponent.h"
namespace acs
{
	LoginSystem::LoginSystem()
	{
		this->mActorComponent = nullptr;
		this->mMasterComponent = nullptr;
	}

	bool LoginSystem::OnInit()
	{
		BIND_SERVER_RPC_METHOD(LoginSystem::Login);
		BIND_SERVER_RPC_METHOD(LoginSystem::Logout);
		this->mApp->GetComponents(this->mLoginComponents);
		this->mActorComponent = this->GetComponent<ActorComponent>();
		this->mMasterComponent = this->GetComponent<MasterComponent>();
		return true;
	}

    int LoginSystem::Login(long long id, const s2s::login::request & request)
	{
		long long playerId = request.user_id();
		Player * player = this->mActorComponent->GetPlayer(playerId);
		if(player == nullptr)
		{
			player = new Player(playerId, (int)id);
			this->mActorComponent->AddPlayer(player);
		}
		for(const auto & actorInfo : request.actors())
		{
			int actorId = actorInfo.second;
			const std::string & name = actorInfo.first;
			if(!this->mActorComponent->HasServer(actorId))
			{
				if(!this->mMasterComponent->SyncServer(actorId))
				{
					LOG_ERROR("not find server : {}", actorId);
					return XCode::NotFoundServerRpcAddress;
				}
			}
			player->AddAddr(name, actorId);
		}

		Lua::LuaModule * luaModule = this->GetLuaModule();
		if(luaModule != nullptr && luaModule->HasFunction("OnLogin"))
		{
			luaModule->Await("OnLogin", playerId);
		}

		help::PlayerLoginEvent::Trigger(playerId);
		return XCode::Ok;
	}

    int LoginSystem::Logout(long long id, const s2s::logout::request& request)
    {
		long long playerId = request.user_id();
    	this->mActorComponent->DelActor(playerId);
		for(ILogin * loginComponent : this->mLoginComponents)
		{
			loginComponent->OnLogout(playerId);
		}

		Lua::LuaModule * luaModule = this->GetLuaModule();
		if(luaModule != nullptr && luaModule->HasFunction("OnLogout"))
		{
			luaModule->Await("OnLogout", playerId);
		}


		help::PlayerLogoutEvent::Trigger(playerId);
		return XCode::Ok;
    }
}