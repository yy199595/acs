#pragma once
#include <XCode/XCode.h>
#include <Network/SocketProxy.h>
namespace GameKeeper
{
    class HttpHandlerBase;

    class HttpComponent;

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

        virtual void Clear();

		virtual SocketType GetSocketType() = 0;

		XCode GetCode() const { return this->mCode; }


    protected:

        virtual void OnWriterAfter(XCode code) = 0;

        virtual void OnReceiveHeadAfter(XCode code) = 0;

		virtual void OnReceiveHeard(asio::streambuf & buf) = 0;

        virtual bool WriterToBuffer(std::ostream &) = 0;

    private:

        void ReadHeardCallback(const asio::error_code &err, size_t size);
        
        void ReceiveHeard();

        void SendHttpMessage();

    protected:
        XCode mCode;
        std::string mAddress;
        SocketProxy *mSocketProxy;
        asio::streambuf mStreamBuf;
        HttpComponent *mHttpComponent;
    private:
        int mCount;
        bool mIsReadBody;

    };
}