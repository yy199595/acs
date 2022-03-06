#pragma once
#include"Rpc.h"
#include"XCode/XCode.h"
#include"Network/SocketProxy.h"
#include<google/protobuf/message.h>
using namespace google::protobuf;
#define TCP_BUFFER_COUNT 1024
#define MAX_DATA_COUNT 1024 * 20 //处理的最大数据
namespace Sentry
{
    class NetworkData
    {
    public:
        NetworkData(char type, std::shared_ptr<Message> message);

    public:
        size_t GetByteSize();
        bool WriteToBuffer(std::string & streamBuffer);
    private:
        char mType;
        std::shared_ptr<Message> mMessage;
    };
}

namespace Sentry
{
    class RpcClient : public std::enable_shared_from_this<RpcClient>
	{
	public:
		explicit RpcClient(std::shared_ptr<SocketProxy> socket, SocketType type);
		virtual ~RpcClient() = default;
	public:
		void StartReceive();
        inline bool IsConnected() { return this->mIsConnect; }
		inline const std::string & GetIp() const { return this->mIp; }
		inline long long GetSocketId() const { return this->mSocketId; }
        inline bool IsOpen() const { return this->mSocketProxy->IsOpen(); }
        std::shared_ptr<SocketProxy> GetSocketProxy() const { return this->mSocketProxy;}
        inline const std::string & GetAddress() const { return this->mSocketProxy->GetAddress(); }

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
        virtual void OnConnect(XCode code) = 0;
        virtual void OnClientError(XCode code) = 0;
        virtual XCode OnRequest(const char * buffer, size_t size) = 0;
		virtual XCode OnResponse(const char * buffer, size_t size) = 0;
        virtual void OnSendData(XCode code, std::shared_ptr<NetworkData> message) = 0;
    protected:
        bool IsCanConnection();
        void SendData(std::shared_ptr<NetworkData> message);
	protected:
        AsioContext & mContext;
        IAsioThread & mNetWorkThread;
        std::shared_ptr<SocketProxy> mSocketProxy;
    private:
		std::string mIp;
        bool mIsCanSendData;
        unsigned short mPort;
		long long mSocketId;
		atomic_bool mIsConnect;
		const SocketType mType;
		long long mLastOperTime;
        std::string mSendBuffer;
        char mReceiveBuffer[TCP_BUFFER_COUNT];
        std::queue<std::shared_ptr<NetworkData>> mWaitSendQueue;
	};
}
