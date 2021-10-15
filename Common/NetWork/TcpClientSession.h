#pragma once

#include <Define/CommonDef.h>
#include <NetWork/SocketEvent.h>

namespace Sentry
{

    enum ETcpErrorType
    {
        ErrNone,
        ErrConnect,
        ErrRead,
        ErrWrite,
    };

    class ITcpEventHandler
    {
    public:
        virtual void Run(ETcpErrorType type, const char * msg, size_t size);
    };

    template<typename T, typename F>
    class TcpEventHandler : public ITcpEventHandler
    {
    public:
        TcpEventHandler(F && f, T * o): _o(o), _f(std::forward<F>(f)) { }

    public:
        void Run(ETcpErrorType type, const char *msg, size_t size) override
        {
            (_o->*_f)(type, msg, size);
        }
    private:
        T * _o;
        std::remove_reference<F> _f;
    };
}


namespace Sentry
{
    class ISessionHandler;

    class TcpClientSession
    {
    public:
        TcpClientSession(AsioContext &io, ISessionHandler *handler, SharedTcpSocket socket);

        TcpClientSession(AsioContext &io, ISessionHandler *handler, std::string name, std::string ip,
                         unsigned short port);

        virtual ~TcpClientSession();

    public:
        bool IsActive();

        inline const std::string &GetIP()
        { return mIp; }

        inline unsigned short GetPort()
        { return mPort; }

        inline const std::string &GetAddress()
        { return mAdress; }

        inline SharedTcpSocket GetSocket()
        { return this->mBinTcpSocket; }

        inline const std::string &GetSessionName()
        { return mSessionName; }

    public:
        bool SendPackage(SharedMessage message);

    public:
        void StartClose();

        bool StartConnect();

        bool StartReceiveMsg();

    private:
        void ReadMessageBody(const size_t allSize);

        void InitMember(const std::string &ip, unsigned short port);

    private:
        std::string mIp;
        std::string mAdress;
        unsigned short mPort;
        AsioContext &mAsioContext;
        SharedTcpSocket mBinTcpSocket;
        AsioTcpEndPoint mSocketEndPoint;

    private:
        SessionType mSessionType;
        std::string mSessionName;
        unsigned int mConnectCount;
        ISessionHandler * mSessionHandler;

    private:
        char *mRecvMsgBuffer;
        unsigned int mRecvBufferSize;
    };

    typedef shared_ptr<TcpClientSession> SharedTcpSession;

}// namespace Sentry