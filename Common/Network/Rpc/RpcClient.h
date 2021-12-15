#pragma once
#include"Rpc.h"
#include"XCode/XCode.h"
#include"Network/SocketProxy.h"
#include<google/protobuf/message.h>
using namespace google::protobuf;
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
        inline bool IsOpen() const { return this->mIsOpen; }
        inline bool IsConnected() { return this->mIsConnect; }
		inline const std::string & GetIp() const { return this->mIp; }
		inline SocketProxy & GetSocketProxy() { return *mSocketProxy; }
		inline long long GetSocketId() const { return this->mSocketId; }
		inline const std::string & GetAddress() const { return this->mAddress; }
	public:
		virtual void Clear();
		SocketType GetSocketType() { return this->mType; }
        long long GetLastOperatorTime() const { return this->mLastOperTime;}
		bool StartConnect(const std::string & ip, unsigned short port, StaticMethod * method = nullptr);
	private:
		void ReceiveHead();
		void ReceiveBody(char type, size_t size);
		void ConnectHandler(const std::string & ip, unsigned short port, StaticMethod * method);
	protected:
        void CloseSocket(XCode code);
        virtual void OnClose(XCode code) = 0;
        virtual void OnConnect(XCode code) = 0;
        virtual void OnSendData(XCode code, const Message *) = 0;
        virtual XCode OnRequest(const char * buffer, size_t size) = 0;
		virtual XCode OnResponse(const char * buffer, size_t size) = 0;
    protected:
        void SendData(char type, const Message * message);
	protected:
		AsioContext & mContext;
		SocketProxy * mSocketProxy;
		NetWorkThread & mNetWorkThread;
	private:
        bool mIsOpen;
		std::string mIp;
		int mConnectCount;
		long long mSocketId;
		std::string mAddress;
		atomic_bool mIsConnect;
		const SocketType mType;
		long long mLastOperTime;
        char mSendBuffer[TCP_BUFFER_COUNT];
        char mReceiveBuffer[TCP_BUFFER_COUNT];
	};
}
