#pragma once
#include<Define/CommonDef.h>
namespace SoEasy
{
	class ConsoleManager;
	class TelnetClientSession : public std::enable_shared_from_this<TelnetClientSession>
	{
	public:
		TelnetClientSession(ConsoleManager * manager, SharedTcpSocket socket);
		~TelnetClientSession() { }
	public:
		void StartRecvMessage();
		void StartWriteMessage(const std::string & message);
	public:
		const std::string & GetAddress() { return mAddress; }
	private:
		void Close();
		void Receive();
	private:
		std::string mAddress;
		AsioContext & mAsioContext;
		SharedTcpSocket mBindSocket;
		ConsoleManager * mSessionManager;
	private:
		asio::streambuf mRecvMsgBuffer;
	};
	typedef std::shared_ptr<TelnetClientSession> SharedTelnetSession;
}