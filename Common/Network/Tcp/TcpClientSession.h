#pragma once
#include<array>
#include <Define/CommonDef.h>
#include <Network/SocketProxy.h>
#define TCP_BUFFER_COUNT 1024

namespace GameKeeper
{
    class TcpClientComponent;

    class TcpClientSession
    {
    public:
        explicit TcpClientSession(TcpClientComponent *component);
        virtual ~TcpClientSession();
	public:
		void SetSocket(SocketProxy * socketProxy);
		bool IsOpen() { return this->mSocketProxy->IsOpen(); }
		const std::string & GetAddress() const { return this->mAddress; }
        virtual SocketType GetSocketType() { return SocketType::RemoteSocket;}
	public:
		void StartClose();
		void StartReceive();
		void StartSendByString(std::string * message);
	public:
		SocketProxy & GetSocketProxy() { return *mSocketProxy; }
    private:
		void CloseSocket();
		void ReceiveMessage();
		void SendByString(std::string * message);
        void ReadMessageBody(const size_t allSize);
	protected:
		std::string mAddress;
		SocketProxy * mSocketProxy;
		TcpClientComponent * mTcpComponent;
    private:
		char *mReceiveMsgBuffer;		
    };

    typedef shared_ptr<TcpClientSession> SharedTcpSession;

}// namespace GameKeeper