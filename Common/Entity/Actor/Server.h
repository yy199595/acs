//
// Created by leyi on 2023/5/23.
//

#ifndef APP_SERVER_H
#define APP_SERVER_H
#include"Actor.h"

namespace Tendo
{
	class Server : public Actor, public std::enable_shared_from_this<Server>
	{
	public:
		explicit Server(int id, const std::string & name);
	public:
		int Send(const std::shared_ptr<Msg::Packet> & message);
		bool GetListen(const std::string & name, std::string & addr);
		void AddListen(const std::string & name, const std::string & addr);
		int GetAddress(const std::string &func, std::string &addr) final;
	public:
		bool OnInit() final;
		void OnRegister(std::string &json) final;
		const std::string & Address() const { return this->mRpcAddress; }
	private:
		std::string mRpc;
		std::string mRpcAddress;
		std::unordered_map<std::string, std::string> mListens;
	};
}


#endif //APP_SERVER_H
