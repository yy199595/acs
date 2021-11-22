#pragma once
#include"Rpc.h"
#include"XCode/XCode.h"
#include"Network/SocketProxy.h"
#define TCP_BUFFER_COUNT 1024
#define MAX_DATA_COUNT 1024 * 20 //处理的最大数据
#define TCP_HEAD unsigned int
namespace GameKeeper
{
	class RpcClient
	{
	public:
		explicit RpcClient(SocketProxy * socket, SocketType type);
		virtual ~RpcClient() = default;
	public:
		void StartReceive();
		inline bool IsConnected() { return this->mIsConnect; }
		inline bool IsOpen() { return this->mSocketProxy->IsOpen(); }
		inline const std::string & GetIp() const { return this->mIp; }
		inline SocketProxy & GetSocketProxy() { return *mSocketProxy; }
		inline long long GetSocketId() const { return this->mSocketId; }
		inline const std::string & GetAddress() const { return this->mAddress; }
	public:
		virtual void Clear();
		SocketType GetSocketType() { return this->mType; }
		bool StartConnect(std::string & ip, unsigned short port, StaticMethod * method = nullptr);
	private:
		void ReceiveHead();
		void ReceiveBody(char type, size_t size);
		void ConnectHandler(std::string & ip, unsigned short port, StaticMethod * method);
	protected:
		virtual void OnConnect(XCode code) = 0;
		virtual void CloseSocket(XCode code) = 0;
		bool AsyncSendMessage(char * buffer, size_t size);
		virtual bool OnRequest(const char * buffer, size_t size) = 0;
		virtual bool OnResponse(const char * buffer, size_t size) = 0;
		virtual void OnSendAfter(XCode code, const char * buffer, size_t size) = 0;
	protected:
		AsioContext & mContext;
		SocketProxy * mSocketProxy;
		NetWorkThread & mNetWorkThread;
	private:
		std::string mIp;
		int mConnectCount;
		long long mSocketId;
		std::string mAddress;
		atomic_bool mIsConnect;
		const SocketType mType;
		long long mLastOperTime;
		char mReceiveBuffer[TCP_BUFFER_COUNT];
	};
}
