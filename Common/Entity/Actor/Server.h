//
// Created by leyi on 2023/5/23.
//

#ifndef APP_SERVER_H
#define APP_SERVER_H
#include"Actor.h"

namespace acs
{
	class Server : public Actor
	{
	public:
		explicit Server(int id, const std::string & name);
		inline bool Equal(long long id) const { return this->GetId() == id; }
		inline bool Equal(const std::string & addr) { return this->mRpcAddress == addr; }
	public:
		int DisConnect();
		int SendMsg(std::unique_ptr<rpc::Packet> message);
		bool GetAddress(const rpc::Packet &request, int &) const final;
		bool GetListen(const std::string & name, std::string & addr) const;
		bool AddListen(const std::string & name, const std::string & addr);
	private:
		bool GetListen(int net, std::string & address) const;
	public:
		bool OnInit() final;
		void EncodeToJson(std::string *json) final;
		int GetSrvId() const { return (int)this->GetId(); }
		const std::string & Address() const { return this->mRpcAddress; }
		int Make(const std::string &func, std::unique_ptr<rpc::Packet> &request) const final ;
	private:
		long long mAppId;
		std::string mRpc;
		std::string mRpcAddress;
		std::unordered_map<std::string, std::string> mListens;
	};
}


#endif //APP_SERVER_H
