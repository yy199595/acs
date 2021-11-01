#pragma once
#include <Network/SessionBase.h>
namespace GameKeeper
{
	class HttpHandlerBase;
	class HttpSessionBase : public SessionBase
	{
	public:
		explicit HttpSessionBase(ISocketHandler *  handler);
		virtual ~HttpSessionBase() = default;
	public:
		void StartSendHttpMessage();
	protected:
		void StartReceive();
		virtual bool WriterToBuffer(std::ostream & os) = 0;
		virtual bool OnReceiveBody(asio::streambuf & buf, const asio::error_code & code) = 0;
		virtual bool OnReceiveHeard(asio::streambuf & buf, size_t  size, const asio::error_code & code) = 0;
    protected:
        virtual void OnSendHttpMessageAfter() { }
        virtual void OnSocketError(const asio::error_code & err);
	private:
		void ReadCallback(const asio::error_code & err, size_t size);
	private:
		int mCount;
		bool mIsReadBody;
		asio::streambuf mStreamBuf;
	};
}