//
// Created by zmhy0073 on 2022/10/13.
//

#include"Login.h"
#include"Entity/Actor/App.h"
#include"Core/Event/IEvent.h"
#include"Lua/Module/LuaModule.h"
#include"Master/Component/MasterComponent.h"
namespace joke
{
	Login::Login()
	{
		this->mActorComponent = nullptr;
		this->mMasterComponent = nullptr;
	}

	bool Login::OnInit()
	{
		BIND_SERVER_RPC_METHOD(Login::OnLogin);
		BIND_SERVER_RPC_METHOD(Login::OnLogout);
		this->mActorComponent = this->mApp->ActorMgr();
		this->mApp->GetComponents(this->mLoginComponents);
		this->mMasterComponent = this->GetComponent<MasterComponent>();
		return true;
	}

    int Login::OnLogin(long long id, const s2s::login::request & request)
	{
		long long playerId = request.user_id();
		Player * player = this->mActorComponent->GetPlayer(playerId);
		if(player == nullptr)
		{
			player = new Player(playerId, id);
			{
				this->mActorComponent->AddPlayer(player);
			}
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
		if(luaModule->HasFunction("_OnLogin"))
		{
			luaModule->Await("_OnLogin", playerId);
		}

		help::PlayerLoginEvent::Trigger(playerId);
		return XCode::Ok;
	}

    int Login::OnLogout(long long id, const s2s::logout::request& request)
    {
		long long playerId = request.user_id();
    	this->mActorComponent->DelActor(playerId);
		for(ILogin * loginComponent : this->mLoginComponents)
		{
			loginComponent->OnLogout(playerId);
		}
		help::PlayerLogoutEvent::Trigger(playerId);
		return XCode::Ok;
    }
}