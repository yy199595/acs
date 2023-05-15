//
// Created by leyi on 2023/5/15.
//

#ifndef APP_SERVER_H
#define APP_SERVER_H

#include"NetUnit.h"
namespace Tendo
{
	class Server : public NetUnit
	{
	public:
		Server() : NetUnit(0) { }
	public:
		int Send(const std::string & addr, const std::string & func);
		int Send(const std::string & addr, const std::string & func, const pb::Message & request);
	public:
		int Send(const std::string & addr, const std::shared_ptr<Msg::Packet> & message);
	public:
		int Call(const std::string & addr, const std::string & func);
		int Call(const std::string & addr, const std::string & func, const pb::Message & request);
		int Call(const std::string & addr, const std::string & func, std::shared_ptr<pb::Message> response);
		int Call(const std::string & addr, const std::string & func, const pb::Message & request, std::shared_ptr<pb::Message> response);
	public:
		bool GetAddr(int targetId, std::string & addr);
		bool GetAddr(const std::string &server, int &targetId) final;
		bool GetAddr(const std::string &server, std::string &addr) final;
	};
}


#endif //APP_SERVER_H
