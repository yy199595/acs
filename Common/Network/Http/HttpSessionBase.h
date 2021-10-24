#pragma once
#include <Network/SessionBase.h>
namespace Sentry
{
	class HttpSessionBase : public SessionBase
	{
	public:
		HttpSessionBase(ISocketHandler *  handler);
		virtual ~HttpSessionBase() { }
	protected:
		void StartReceive();
		virtual void OnReceive(asio::streambuf & stream, const asio::error_code & err) = 0;
	private:
		void ReadCallback(const asio::error_code & err, size_t size);
	protected:
		asio::streambuf mStreamBuf;
	};
}