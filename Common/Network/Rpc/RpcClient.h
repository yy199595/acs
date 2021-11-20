#pragma once
#include"Rpc.h"
#include"XCode/XCode.h"
#include"Network/SocketProxy.h"
#define TCP_BUFFER_COUNT 1024
#define MAX_DATA_COUNT 1024 * 20 //接受最大数据包
namespace GameKeeper
{
	class RpcClient
	{
	public:
		RpcClient(SocketProxy * socket);
		virtual ~RpcClient() { }
	public:
		void StartReceive();
		
		bool IsOpen() { return this->mSocketProxy->IsOpen(); }
		const std::string & GetIp() const { return this->mIp; }
		SocketProxy & GetSocketProxy() { return *mSocketProxy; }
		long long GetSocketId() const { return this->mSocketId; }
		const std::string & GetAddress() const { return this->mAddress; }
	public:
		virtual SocketType GetSocketType() = 0;
	protected:
		void ReceiveHead();
		void ReceiveBody(char type, size_t size);
		bool AsyncSendMessage(char * buffer, size_t size);
	protected:
		virtual void CloseSocket(XCode code) = 0;
		virtual bool OnRequest(char * buffer, size_t size) = 0;
		virtual bool OnResponse(char * buffer, size_t size) = 0;
		
	protected:
		AsioContext & mContext;
		SocketProxy * mSocketProxy;
		NetWorkThread & mNetWorkThread;
	private:
		std::string mIp;
		char * mRecvBuffer;
		long long mSocketId;
		std::string mAddress;	
		long long mLastOperTime;
	};
}