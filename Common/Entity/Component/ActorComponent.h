//
// Created by yjz on 2023/5/17.
//

#ifndef _ACTORMGRCOMPONENT_H_
#define _ACTORMGRCOMPONENT_H_
#include"Component.h"
#include"Entity/Actor/ServerActor.h"
#include"Entity/Actor/PlayerActor.h"
namespace Lua
{
	class LuaModule;
};
namespace Tendo
{
	class ActorComponent : public Component
	{
	 public:
		Actor * GetActor(long long id);
		PlayerActor * GetPlayer(long long playerId);
		ServerActor * GetServer(long long serverId);
	public:
		bool AddPlayer(std::shared_ptr<PlayerActor> player);
		bool AddServer(std::shared_ptr<ServerActor> server);
		void GetServers(std::vector<ServerActor *> & servers);
		bool AddServer(std::shared_ptr<ServerActor> server, const std::string & json);
	public:
		ServerActor * Random(const std::string & name);
	 public:
		bool DelActor(long long id);
	private:
		bool LateAwake() final;
		bool AddRandomActor(const std::string & name, Actor * actor);
	private:
		Lua::LuaModule * mLuaModule;
		std::unordered_map<long long, std::shared_ptr<ServerActor>> mServers;
		std::unordered_map<long long, std::shared_ptr<PlayerActor>> mPlayers;
		std::unordered_map<std::string, std::vector<long long>> mActorNames;
	};
}

#endif //_ACTORMGRCOMPONENT_H_
