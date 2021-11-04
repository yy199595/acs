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
		explicit HttpSessionBase(HttpClientComponent *  component);
		virtual ~HttpSessionBase() = default;
	public:
        void StartReceiveBody();
        void StartReceiveHeard();
		void StartSendHttpMessage();
        bool IsReadBody() const { return this->mIsReadBody;}
        const std::string & GetPath() const { return this->mPath;}
		virtual void SetSocketProxy(SocketProxy * socketProxy) = 0;	
		const std::string & GetAddress() const { return this->mAddress; }
		const std::string & GetVersion() const { return this->mVersion; }
		NetWorkThread &  GetThread() { return this->mSocketProxy->GetThread(); }
	protected:
        virtual void OnWriteAfter(XCode code) = 0;
        virtual void OnReceiveBodyAfter(XCode code) = 0;
        virtual void OnReceiveHeardAfter(XCode code) = 0;
        virtual bool WriterToBuffer(std::ostream & os) = 0;
		virtual void OnReceiveBody(asio::streambuf & buf) = 0;
        virtual bool OnReceiveHeard(asio::streambuf & buf, size_t  size) = 0;
	private:
        void ReadBodyCallback(const asio::error_code & err, size_t size);
        void ReadHeardCallback(const asio::error_code & err, size_t size);
	private:
		void ReceiveBody();
		void ReceiveHeard();
    protected:
        std::string mPath;
        std::string mVersion;
		std::string mAddress;
		SocketProxy * mSocketProxy;
		HttpClientComponent * mHttpComponent;
	private:
		int mCount;
		bool mIsReadBody;
		asio::streambuf mStreamBuf;
	};
}