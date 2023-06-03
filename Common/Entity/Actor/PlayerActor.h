//
// Created by leyi on 2023/5/15.
//

#ifndef APP_PLAYER_H
#define APP_PLAYER_H
#include"Entity/Actor/Actor.h"

namespace Tendo
{
	class PlayerActor : public Actor
	{
	public:
		PlayerActor(long long playerId, int gate);
	public:
		bool OnInit() final;
		void OnRegister(std::string *json) final;
		bool DelAddr(const std::string & server);
		void GetActors(std::vector<int> & actors) const;
		void AddAddr(const std::string & server, int id);
		int GetAddress(const std::string &func, std::string &addr) final;
	public:
		int SendToClient(const std::string & func);
		int SendToClient(const std::shared_ptr<Msg::Packet> & message);
		int SendToClient(const std::string & func, const pb::Message & request);
	private:
		int mGateId;
		class ActorMgrComponent * mActorComponent;
		std::unordered_map<std::string, int> mServerAddrs;
	};
}


#endif //APP_PLAYER_H
