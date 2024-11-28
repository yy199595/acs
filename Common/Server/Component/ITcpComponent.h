//
// Created by 64658 on 2024/11/19.
//

#ifndef APP_ITCPCOMPONENT_H
#define APP_ITCPCOMPONENT_H
#include <string>
#include"Network/Tcp/Socket.h"

namespace acs
{

	class ITcpListen
	{
	public:
		virtual bool OnListen(tcp::Socket* socket) = 0;
	};

	namespace proto_type
	{
		constexpr int tcp = 1;
		constexpr int udp = 2;
	}

	struct ListenConfig
	{
	public:
		int Port;            //监听
		int MaxConn;        //最大连接数
		std::string ip;
		int ProtoType = proto_type::tcp;
#ifdef __ENABLE_OPEN_SSL__
		std::string Key;    //私钥
		std::string Cert;    //证书地址
		std::string Verify;
#endif
		std::string Name;    //名字
		std::string Addr;  //地址 xxx://xxx:xxx
		std::string Component; //处理消息的component
		std::string ProtoName;
	};

	class INetListen
	{
	public:
		virtual bool StopListen() = 0;
		virtual bool StartListen(const ListenConfig & listen) = 0;
	};
}

#endif //APP_ITCPCOMPONENT_H
