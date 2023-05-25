//
// Created by yjz on 2023/5/17.
//

#ifndef _ACTORMGRCOMPONENT_H_
#define _ACTORMGRCOMPONENT_H_
#include"Component.h"
#include"Entity/Actor/Server.h"
#include"Entity/Actor/Player.h"
namespace Tendo
{
	class ActorMgrComponent : public Component
	{
	 public:
		Actor * GetActor(long long id);
		Player * GetPlayer(long long playerId);
		Server * GetServer(long long serverId);
	public:
		bool AddPlayer(std::shared_ptr<Player> player);
		bool AddServer(std::shared_ptr<Server> server);
		void GetServers(std::vector<Server *> & servers);
	public:
		Server * Random(const std::string & name);
	 public:
		bool DelActor(long long id);
	private:
		bool AddRandomActor(const std::string & name, Actor * actor);
	private:
		std::unordered_map<long long, std::shared_ptr<Server>> mServers;
		std::unordered_map<long long, std::shared_ptr<Player>> mPlayers;
		std::unordered_map<std::string, std::vector<long long>> mActorNames;
	};
}

#endif //_ACTORMGRCOMPONENT_H_
