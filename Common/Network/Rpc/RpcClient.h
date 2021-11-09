#pragma once
#include<XCode/XCode.h>
#include <Define/CommonDef.h>
#include <Network/SocketProxy.h>
#define TCP_BUFFER_COUNT 1024

namespace GameKeeper
{
    class RpcComponent;

    class RpcClient
    {
    public:
        explicit RpcClient(RpcComponent *component);
        virtual ~RpcClient();
	public:
		void SetSocket(SocketProxy * socketProxy);
		bool IsOpen() { return this->mSocketProxy->IsOpen(); }
		long long GetLastOperTime() const { return this->mLastOperTime; }
		const std::string & GetAddress() const { return this->mAddress; }
        virtual SocketType GetSocketType() { return SocketType::RemoteSocket;}		
	public:
		void StartClose();
		void StartReceive();
		void StartSendByString(std::string * message);
	public:
		SocketProxy & GetSocketProxy() { return *mSocketProxy; }
    private:
		void ReceiveMessage();
		void CloseSocket(XCode code);
		void SendByString(std::string * message);
        void ReadMessageBody(const size_t allSize);
	protected:
		std::string mAddress;
		SocketProxy * mSocketProxy;
		RpcComponent * mTcpComponent;
    private:
		char *mReceiveMsgBuffer;
		long long mLastOperTime;
    };

    typedef shared_ptr<RpcClient> SharedTcpSession;

}// namespace GameKeeper