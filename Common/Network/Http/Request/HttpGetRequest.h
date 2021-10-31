#pragma once
#include <XCode/XCode.h>
#include <Network/Http/HttpHandlerBase.h>
namespace Sentry
{
	class HttpClientComponent;
	class HttpGetRequest : public HttpHandlerBase
	{
	public:
		HttpGetRequest(HttpClientComponent * component);
		~HttpGetRequest() = default;
	public:
		XCode Get(const std::string & url, std::string & response);
	protected:
		bool WriterToBuffer(std::ostream & os);
		bool OnSessionError(const asio::error_code & code);
		bool OnReceiveBody(asio::streambuf & buf, const asio::error_code & code) override;
		bool OnReceiveHeard(asio::streambuf & buf, const asio::error_code & code) override;
	private:
		XCode mCode;
		int mHttpCode;
		std::string mHttpError;
		std::string * mResponse;
		std::string mHttpVersion;
	private:
		std::string mHost;
		std::string mPort;
		std::string mPath;
		unsigned int mCorId;
		class HttpClientComponent * mHttpComponent;
	};
}