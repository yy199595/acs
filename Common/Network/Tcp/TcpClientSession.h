#pragma once

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
	protected:
		void OnSessionEnable() override;
    private:
        void StartReceive();
        void ReadMessageBody(const size_t allSize);
    private:
        char *mRecvMsgBuffer;
        unsigned int mRecvBufferSize;
    };

    typedef shared_ptr<TcpClientSession> SharedTcpSession;

}// namespace Sentry