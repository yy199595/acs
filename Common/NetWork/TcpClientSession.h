#pragma once

#include <Define/CommonDef.h>
#include <NetWork/SessionBase.h>

namespace Sentry
{

    enum ETcpErrorType
    {
        ErrNone,
        ErrConnect,
        ErrRead,
        ErrWrite,
    };
}


namespace Sentry
{
    class ISocketHandler;

    class TcpClientSession : public SessionBase
    {
    public:
        TcpClientSession(ISocketHandler *handler);
     
        virtual ~TcpClientSession();

	public:
		const std::string &GetAddress() final { return this->mAdress; }     

        inline const std::string &GetSessionName()
        { return mSessionName; }

    public:
    
	protected:
		void OnStartReceive();

		void OnStartConnect(std::string name, std::string ip, unsigned short port);

    private:
        void ReadMessageBody(const size_t allSize);

    private:
		std::string mAdress;
        std::string mSessionName;
        unsigned int mConnectCount;      
    private:
        char *mRecvMsgBuffer;
        unsigned int mRecvBufferSize;
    };

    typedef shared_ptr<TcpClientSession> SharedTcpSession;

}// namespace Sentry