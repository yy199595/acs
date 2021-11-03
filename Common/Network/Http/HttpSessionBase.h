#pragma once
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
	protected:
        virtual void OnWriteAfter() = 0;
		virtual bool WriterToBuffer(std::ostream & os) = 0;
		virtual void OnReceiveBody(asio::streambuf & buf) = 0;
        virtual void OnSocketError(const asio::error_code & err) = 0;
        virtual bool OnReceiveHeard(asio::streambuf & buf, size_t  size) = 0;
	private:
        void ReadBodyCallback(const asio::error_code & err, size_t size);
        void ReadHeardCallback(const asio::error_code & err, size_t size);
	private:
		int mCount;
		bool mIsReadBody;
		asio::streambuf mStreamBuf;
	};
}