//
// Created by leyi on 2023/5/15.
//

#ifndef APP_PLAYER_H
#define APP_PLAYER_H
#include"Entity/Actor/Actor.h"

namespace Tendo
{
	class Player : public Actor
	{
	public:
		Player(long long playerId, const std::string & gate);
	public:
		bool DelAddr(const std::string & server);
		void AddAddr(const std::string & server, long long id);
		int GetAddress(const std::string &func, std::string &addr) final;
	public:
		int SendToClient(const std::string & func);
		int SendToClient(const std::string & func, const pb::Message & request);
	private:
		class ActorMgrComponent * mActorComponent;
		std::unordered_map<std::string, long long> mServerAddrs;
	};
}


#endif //APP_PLAYER_H
