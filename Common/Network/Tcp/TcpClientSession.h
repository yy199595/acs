#pragma once
#include<array>
#include <Define/CommonDef.h>
#include <Network/SessionBase.h>


namespace Sentry
{
    class ISocketHandler;

    class TcpClientSession : public SessionBase
    {
    public:
        TcpClientSession(ISocketHandler *handler);     
        virtual ~TcpClientSession();
	public:
		bool StartConnect(const std::string & name, const std::string & ip, unsigned short port);
	protected:
		void OnSessionEnable() override;
    private:
        void StartReceive();
        void ReadMessageBody(const size_t allSize);
		void ConnectCallback(const asio::error_code &err);
		void ConnectHandler(const std::string & ip, unsigned short port);
    private:
        char *mReceiveMsgBuffer;
        unsigned int mReceiveBufferSize;
    };

    typedef shared_ptr<TcpClientSession> SharedTcpSession;

}// namespace Sentry