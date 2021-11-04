#pragma once
#include <XCode/XCode.h>
#include <Network/SessionBase.h>
namespace GameKeeper
{
	class HttpHandlerBase;
	class HttpSessionBase : public SessionBase
	{
	public:
		explicit HttpSessionBase(ISocketHandler *  handler, const std::string & name);
		virtual ~HttpSessionBase() = default;
	public:
        void StartReceiveBody();
        void StartReceiveHeard();
		void StartSendHttpMessage();
        bool IsReadBody() const { return this->mIsReadBody;}
        const std::string & GetPath() const { return this->mPath;}
        const std::string & GetVersion() const { return this->mVersion;}
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
    protected:
        std::string mPath;
        std::string mVersion;
	private:
		int mCount;
		bool mIsReadBody;
		asio::streambuf mStreamBuf;
	};
}