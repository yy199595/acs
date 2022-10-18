#pragma once
#include"Client/Rpc.h"
#include"Tcp/TcpContext.h"
#include"Client/Message.h"
#include"Source/TaskSource.h"
#include"Component/IComponent.h"
#include"Coroutine/CoroutineLock.h"
#include<google/protobuf/message.h>
using namespace Tcp;
using namespace google::protobuf;

namespace Sentry
{
	class InnerNetClient : public Tcp::TcpContext
	{
	 public:
		explicit InnerNetClient(IRpc<Rpc::Data> * component, std::shared_ptr<SocketProxy> socket);
		~InnerNetClient() override = default;
	 public:
		void StartClose();
		void StartReceive();
        void SendData(std::shared_ptr<Rpc::Data> message);
    private:
        void CloseSocket(XCode code);
        void OnConnect(const asio::error_code &error, int count) final;
        void OnReceiveMessage(const asio::error_code &code, std::istream & is, size_t) final;
        void OnSendMessage(const asio::error_code &code, std::shared_ptr<ProtoMessage> message) final;
	private:
        std::string mSrvName;
        std::string mUserName;
        std::string mPassword;
        Tcp::DecodeState mState;
        std::string mRpcLocation;
        std::string mHttpLocation;
        IRpc<Rpc::Data> * mComponent;
        std::shared_ptr<Rpc::Data> mMessage;
        std::shared_ptr<asio::steady_timer> mTimer;
	};
}// namespace Sentry