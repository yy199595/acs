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
		explicit RpcClient(std::shared_ptr<SocketProxy> socket, SocketType type);
		virtual ~RpcClient() = default;
	public:
		void StartReceive();
        inline bool IsOpen() const { return this->mIsOpen; }
        inline bool IsConnected() { return this->mIsConnect; }
		inline const std::string & GetIp() const { return this->mIp; }
		inline long long GetSocketId() const { return this->mSocketId; }
		inline const std::string & GetAddress() const { return this->mAddress; }
        std::shared_ptr<SocketProxy> GetSocketProxy() const { return this->mSocketProxy;}
	public:
		virtual void Clear();
		SocketType GetSocketType() { return this->mType; }
        bool StartConnect(const std::string & ip, unsigned short port);
        long long GetLastOperatorTime() const { return this->mLastOperTime;}
	private:
		void ReceiveHead();
		void ReceiveBody(char type, size_t size);
		void ConnectHandler(const std::string & ip, unsigned short port);
	protected:
        void CloseSocket(XCode code);
        virtual void OnClose(XCode code) = 0;
        virtual void OnConnect(XCode code) = 0;
        virtual XCode OnRequest(const char * buffer, size_t size) = 0;
		virtual XCode OnResponse(const char * buffer, size_t size) = 0;
        virtual void OnSendData(XCode code, std::shared_ptr<Message> ) = 0;
    protected:
        void ReConnection();
        bool IsCanConnection();
        void SendData(char type, std::shared_ptr<Message> message);
	protected:
		AsioContext & mContext;
		NetWorkThread & mNetWorkThread;
        std::shared_ptr<SocketProxy> mSocketProxy;
    private:
        bool mIsOpen;
		std::string mIp;
        unsigned short mPort;
		long long mSocketId;
		std::string mAddress;
		atomic_bool mIsConnect;
		const SocketType mType;
		long long mLastOperTime;
        char mSendBuffer[TCP_BUFFER_COUNT];
        char mReceiveBuffer[TCP_BUFFER_COUNT];
	};
}
