#pragma once
#include<Protocol/com.pb.h>
#include<NetWork/SocketEvent.h>

using namespace google::protobuf;
namespace SoEasy
{
	class TcpProxySession
	{
	public:
		TcpProxySession(const std::string & address);
		TcpProxySession(const std::string & name, const std::string & address);
	public:
		const std::string & GetName() { return this->mName; }
		const std::string & GetAddress() { return this->mAddress; }
	public:
		bool SendMessageData(PB::NetWorkPacket * messageData);
	public:
		bool Notice(const std::string &service, const std::string &method); //不回应
		bool Notice(const std::string &service, const std::string &method, const Message & request); //不回应
	public:
		XCode Invoke(const std::string &service, const std::string &method);
		XCode Invoke(const std::string &service, const std::string &method, const Message & request);
	public: // c++ 使用
		XCode Call(const std::string &service, const std::string &method, Message &response);
		XCode Call(const std::string &service, const std::string &method, const Message & request, Message &response);
	private:
		std::string mName;
		std::string mAddress;
		SessionType mSessionType;
		class CoroutineManager * mCorManager;
		class NetSessionManager * mNetManager;
		class NetProxyManager * mNetProxyManager;
	};
}