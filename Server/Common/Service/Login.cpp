//
// Created by zmhy0073 on 2022/10/13.
//

#include"Login.h"
#include"Entity/Actor/App.h"
#include"Lua/Module/LuaModule.h"
#include"Rpc/Component/InnerNetComponent.h"
namespace Tendo
{
	bool Login::OnInit()
	{
		BIND_COMMON_RPC_METHOD(Login::OnLogin);
		BIND_COMMON_RPC_METHOD(Login::OnLogout);
		this->mActorComponent = this->mApp->ActorMgr();
		return true;
	}

    int Login::OnLogin(long long id, const s2s::login::request & request)
	{
		long long playerId = request.user_id();
		PlayerActor * player = this->mActorComponent->GetPlayer(playerId);
		if(player == nullptr)
		{
			std::shared_ptr<PlayerActor> newPlayer = std::make_shared<PlayerActor>(playerId, id);
			{
				player = newPlayer.get();
				this->mActorComponent->AddPlayer(newPlayer);
			}
		}
		for(const auto & actorInfo : request.actors())
		{
			player->AddAddr(actorInfo.first, actorInfo.second);
		}
		Lua::LuaModule * luaModule = this->GetLuaModule();
		if(luaModule != nullptr)
		{
			luaModule->Await("OnPlayerLogin", playerId);
		}
		return XCode::Successful;
	}

    int Login::OnLogout(long long id, const s2s::logout::request& request)
    {
		long long playerId = request.user_id();
    	this->mActorComponent->DelActor(playerId);
		Lua::LuaModule * luaModule = this->GetLuaModule();
		if(luaModule != nullptr)
		{
			luaModule->Await("OnPlayerLogout", playerId);
		}
		return XCode::Successful;
    }

}