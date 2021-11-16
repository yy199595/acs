#pragma once
#include<Rpc.h>
#include<XCode/XCode.h>
#include<Define/CommonDef.h>
#include<SocketProxy.h>
#define TCP_BUFFER_COUNT 1024
#define MAX_DATA_COUNT 1204 * 10 //接受最大数据包
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
		const std::string & GetIp() const { return this->mIp; }
		long long GetLastOperTime() const { return this->mLastOperTime; }
		const std::string & GetAddress() const { return this->mAddress; }
        virtual SocketType GetSocketType() { return SocketType::RemoteSocket;}		
	public:
		void StartClose();
		void StartReceive();
		void StartSendProtocol(char type, const Message * message);
	public:
		SocketProxy & GetSocketProxy() { return *mSocketProxy; }
    private:
		void ReceiveMessage();
		void CloseSocket(XCode code);
        void ReadMessageBody(unsigned int allSize, int type);
		void SendProtocol(char type, const Message * message);

    protected:
		std::string mIp;
		std::string mAddress;
		SocketProxy * mSocketProxy;
		RpcComponent * mTcpComponent;
    private:
		char *mReceiveMsgBuffer;
		long long mLastOperTime;
		std::queue<const Message *> mMessageQueue;
    };

    typedef shared_ptr<RpcClient> SharedTcpSession;

}// namespace GameKeeper