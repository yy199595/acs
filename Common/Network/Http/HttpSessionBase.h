#pragma once
#include <XCode/XCode.h>
#include <SocketProxy.h>
namespace GameKeeper
{
    class HttpHandlerBase;

    class HttpClientComponent;

    class HttpSessionBase
    {
    public:
        explicit HttpSessionBase();

        virtual ~HttpSessionBase();

    public:

        void StartReceiveHead();

        void StartSendHttpMessage();

        const std::string &GetAddress() const { return this->mAddress; }

        NetWorkThread &GetThread() { return this->mSocketProxy->GetThread(); }
	public:

		virtual SocketType GetSocketType() = 0;

    protected:

        virtual void OnWriterAfter(XCode code) = 0;

        virtual void OnComplete(XCode code) = 0;

		virtual bool OnReceiveHead(asio::streambuf & buf) = 0;

        virtual bool OnReceiveBody(asio::streambuf & buf) = 0;

        virtual void WriterToBuffer(std::ostream &) = 0;
        
    private:

        void OnRecvHead(const asio::error_code &err, size_t size);
        void OnRecvBody(const asio::error_code &err, size_t size);


        void ReceiveHead();

        void ReceiveBody();
        void SendHttpMessage();

    protected:
        std::string mAddress;
        SocketProxy *mSocketProxy;
    private:
        int mCount;
        bool mIsReadBody;
        asio::streambuf mStreamBuf;
    };
}