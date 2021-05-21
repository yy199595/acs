#pragma once
#include<Define/CommonDef.h>
namespace SoEasy
{
	class CommandManager;
	class TelnetClientSession
	{
	public:
		TelnetClientSession(CommandManager * manager, SharedTcpSocket socket);
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
		CommandManager * mSessionManager;
	private:
		asio::streambuf mRecvMsgBuffer;
	};
}