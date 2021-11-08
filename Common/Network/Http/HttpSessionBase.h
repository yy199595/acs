#pragma once
#include <XCode/XCode.h>
#include <Network/SocketProxy.h>
namespace GameKeeper
{
    class HttpHandlerBase;

    class HttpClientComponent;

    class HttpSessionBase
    {
    public:
        explicit HttpSessionBase(HttpClientComponent *component);

        virtual ~HttpSessionBase();

    public:

        void StartReceiveHeard();

        void StartSendHttpMessage();

        const std::string &GetAddress() const { return this->mAddress; }

        NetWorkThread &GetThread() { return this->mSocketProxy->GetThread(); }
	public:

        virtual void Clear();

		virtual SocketType GetSocketType() = 0;

    protected:
		virtual HttpHandlerBase * GetHandler() = 0;

        virtual void OnWriterAfter(XCode code) = 0;

        virtual void OnReceiveHeardAfter(XCode code) = 0;

		virtual bool OnReceiveHeard(asio::streambuf & buf) = 0;

    private:

        void ReadHeardCallback(const asio::error_code &err, size_t size);
        
        void ReceiveHeard();

    protected:
        XCode mCode;
        std::string mAddress;
        SocketProxy *mSocketProxy;
        asio::streambuf mStreamBuf;
        HttpClientComponent *mHttpComponent;
    private:
        int mCount;
        bool mIsReadBody;

    };
}