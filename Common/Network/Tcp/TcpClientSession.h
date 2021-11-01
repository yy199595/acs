#pragma once
#include<array>
#include <Define/CommonDef.h>
#include <Network/SessionBase.h>


namespace GameKeeper
{
    class ISocketHandler;

    class TcpClientSession : public SessionBase
    {
    public:
        TcpClientSession(ISocketHandler *handler);
        virtual ~TcpClientSession();
	public:
        virtual SocketType GetSocketType() override { return SocketType::RemoteSocket;}

	protected:
        void StartReceive();
		void OnSessionEnable() override;
    private:
        void ReadMessageBody(const size_t allSize);
    private:
        char *mReceiveMsgBuffer;
        unsigned int mReceiveBufferSize;
    };

    typedef shared_ptr<TcpClientSession> SharedTcpSession;

}// namespace GameKeeper