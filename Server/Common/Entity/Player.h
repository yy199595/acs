//
// Created by leyi on 2023/5/15.
//

#ifndef APP_PLAYER_H
#define APP_PLAYER_H
#include"Entity/Actor/Actor.h"

namespace acs
{
	class Player final : public Actor
	{
	public:
		Player(long long playerId, int gate, int sockId);
	public:
		inline int GetGateID() const { return this->mGateId; }
		inline int GetClientID() const { return this->mSockId; }
	public:
		void Logout();
		bool OnInit() final;
		void GetActors(std::vector<int> & actors) const;
		bool GetServerId(const std::string & srv, int & id) const;
		bool GetAddress(const rpc::Message &request, int & id) const final;
	public:
		bool DelServer(const std::string & server);
		void AddServer(const std::string & server, int id);
	protected:
		std::unique_ptr<rpc::Message> Make(const std::string &func) const final;
	private:
		int mGateId;
		int mSockId;
		class NodeComponent * mActor;
		std::vector<std::pair<std::string, int>> mServerAddrs;
	};
}


#endif //APP_PLAYER_H
