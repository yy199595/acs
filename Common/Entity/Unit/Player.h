//
// Created by leyi on 2023/5/15.
//

#ifndef APP_PLAYER_H
#define APP_PLAYER_H
#include "NetUnit.h"

namespace Tendo
{
	class Player : public NetUnit
	{
	public:
		Player(long long id);
	public:
		int Send(const std::string & func);
		int Send(const std::string & func, const pb::Message & request);
	public:
		int Call(const std::string & func, const pb::Message & request);
		int Call(const std::string & func, const pb::Message & request, std::shared_ptr<pb::Message> response);
	public:
		int BroadCast(const std::string & func);
		int BroadCast(const std::string & func, const pb::Message & request);
	public:
		bool DelAddr(const std::string & server);
		void AddAddr(const std::string & server, int id);
		bool GetAddr(const std::string &server, int &targetId) final;
		bool GetAddr(const std::string &server, std::string &addr) final;
	private:
		int MakeRequest(const std::string & func, const pb::Message * message, std::string & addr, std::shared_ptr<Msg::Packet> & request);
	private:
		std::unordered_map<std::string, int> mServerAddrs;
	};
}


#endif //APP_PLAYER_H
