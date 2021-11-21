#pragma once
#include"Rpc.h"
#include"XCode/XCode.h"
#include"Network/SocketProxy.h"
#define TCP_BUFFER_COUNT 1024
#define MAX_DATA_COUNT 1024 * 20 //处理的最大数据
namespace GameKeeper
{
	class RpcClient
	{
	public:
		explicit RpcClient(SocketProxy * socket, SocketType type);
		virtual ~RpcClient() = default;
	public:
		void StartReceive();
        bool IsConnected() { return this->mIsConnect; }
		bool IsOpen() { return this->mSocketProxy->IsOpen(); }
		const std::string & GetIp() const { return this->mIp; }
		SocketProxy & GetSocketProxy() { return *mSocketProxy; }
		long long GetSocketId() const { return this->mSocketId; }
		const std::string & GetAddress() const { return this->mAddress; }
	public:
        SocketType GetSocketType() { return this->mType;}
        bool StartConnect(std::string & ip, unsigned short port, StaticMethod * method = nullptr);
    private:
        void ConnectHandler(std::string & ip, unsigned short port, StaticMethod * method);
	protected:
		void ReceiveHead();
		void ReceiveBody(char type, size_t size);
		bool AsyncSendMessage(char * buffer, size_t size);
	protected:
        virtual void OnConnect(XCode code) = 0;
        virtual void CloseSocket(XCode code) = 0;
		virtual bool OnRequest(const char * buffer, size_t size) = 0;
		virtual bool OnResponse(const char * buffer, size_t size) = 0;
		
	protected:
		AsioContext & mContext;
		SocketProxy * mSocketProxy;
		NetWorkThread & mNetWorkThread;
	private:
        std::string mIp;
        int mConnectCount;
        char * mRecvBuffer;
		long long mSocketId;
		std::string mAddress;
        atomic_bool mIsConnect;
        const SocketType mType;
		long long mLastOperTime;
	};
}