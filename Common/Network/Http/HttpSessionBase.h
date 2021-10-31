#pragma once
#include <Network/SessionBase.h>
namespace Sentry
{
	class HttpHandlerBase;
	class HttpSessionBase : public SessionBase
	{
	public:
		HttpSessionBase(ISocketHandler *  handler);
		virtual ~HttpSessionBase() { }
	public:
		void StartSendHttpMessage();
	protected:
		void StartReceive();
		virtual bool WriterToBuffer(std::ostream & os) = 0; // 返回true 表示发送完毕了
		virtual bool OnReceiveBody(asio::streambuf & buf, const asio::error_code & code) = 0;
		virtual bool OnReceiveHeard(asio::streambuf & buf, const asio::error_code & code) = 0;
	private:
		void ReadCallback(const asio::error_code & err, size_t size);
	private:
		int mCount;
		bool mIsReadBody;
		asio::streambuf mStreamBuf;
	};
}