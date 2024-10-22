//
// Created by leyi on 2023/5/15.
//

#ifndef APP_PLAYER_H
#define APP_PLAYER_H
#include"Entity/Actor/Actor.h"

namespace acs
{
	class Player : public Actor
	{
	public:
		Player(long long playerId, int gate);
	public:
		int GetGateID() const { return this->mGateId; }
	public:
		bool OnInit() final;
		void EncodeToJson(std::string *json) final;
		bool DelAddr(const std::string & server);
		void GetActors(std::vector<int> & actors) const;
		void AddAddr(const std::string & server, int id);
		bool GetServerId(const std::string & srv, int & id) const;
		bool GetAddress(const rpc::Packet &request, int & id) const final;
	protected:
		int Make(const std::string &func, std::unique_ptr<rpc::Packet> &request) const final;
	private:
		int mGateId;
		class ActorComponent * mActorComponent;
		std::unordered_map<std::string, int> mServerAddrs;
	};
}


#endif //APP_PLAYER_H
