#pragma once
#include "HttpLocalRequest.h"
namespace Sentry
{
	class HttpClientComponent;
	class HttpLolcalGetRequest : public HttpLocalRequest
	{
	public:
		explicit HttpLolcalGetRequest(HttpClientComponent * component);
		~HttpLolcalGetRequest() override = default;
	public:
		XCode Get(const std::string & url, std::string & response);
	protected:
		bool WriterToBuffer(std::ostream & os) override;
        bool OnReceiveBody(asio::streambuf & buf) override;
	private:
		std::string * mResponse;
	};
}