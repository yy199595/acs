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
        void StartReceiveBody();

        void StartReceiveHeard();

        void StartSendHttpMessage();

        const std::string &GetAddress() const { return this->mAddress; }

        NetWorkThread &GetThread() { return this->mSocketProxy->GetThread(); }
	public:

		//virtual const std::string & GetMethod() = 0;

		virtual void SetSocketProxy(SocketProxy *socketProxy) = 0;
    protected:
		virtual HttpHandlerBase * GetHandler() = 0;

		virtual bool OnReceiveHeard(asio::streambuf & buf) = 0;

    private:
 
        void ReadBodyCallback(const asio::error_code &err, size_t size);

        void ReadHeardCallback(const asio::error_code &err, size_t size);

    private:
        void ReceiveBody();

        void ReceiveHeard();

		
    protected:
        std::string mAddress;
        SocketProxy *mSocketProxy;
        HttpClientComponent *mHttpComponent;
    private:
        int mCount;
        bool mIsReadBody;
        asio::streambuf mStreamBuf;
    };
}