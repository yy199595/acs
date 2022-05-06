#pragma once
#include"Network/Rpc.h"
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
		bool WriteToBuffer(std::string& streamBuffer);
	 private:
		char mType;
		std::shared_ptr<Message> mMessage;
	};
}

namespace Sentry
{
	class RpcClientContext : public std::enable_shared_from_this<RpcClientContext>
	{
	 public:
		explicit RpcClientContext(std::shared_ptr<SocketProxy> socket, SocketType type);
		virtual ~RpcClientContext() = default;
	 public:
		void StartReceive();
		inline bool IsConnected()
		{
			return this->mIsConnect;
		}
		std::shared_ptr<SocketProxy> GetSocketProxy() const
		{
			return this->mSocketProxy;
		}
		inline const std::string& GetAddress() const
		{
			return this->mSocketProxy->GetAddress();
		}

	 public:
		virtual void Clear();
		SocketType GetSocketType()
		{
			return this->mType;
		}
		long long GetLastOperatorTime() const
		{
			return this->mLastOperTime;
		}
	 private:
		void ReceiveHead();
		void ConnectHandler();
		void ReceiveBody(char type, size_t size);
	 protected:
		bool StartConnect();
		virtual void OnConnect(XCode code) = 0;
		virtual void OnClientError(XCode code) = 0;
		virtual bool OnReceiveMessage(char type, const char * buffer, size_t size) = 0;
		virtual void OnSendData(XCode code, std::shared_ptr<NetworkData> message) = 0;
	 protected:
		bool IsCanConnection();
		void SendData(std::shared_ptr<NetworkData> message);
	 protected:
		AsioContext& mContext;
		IAsioThread& mNetWorkThread;
		std::shared_ptr<SocketProxy> mSocketProxy;
	 private:
		bool mIsCanSendData;
		atomic_bool mIsConnect;
		const SocketType mType;
		long long mLastOperTime;
		std::string mSendBuffer;
		char mReceiveBuffer[TCP_BUFFER_COUNT];
		std::queue<std::shared_ptr<NetworkData>> mWaitSendQueue;
	};
}
