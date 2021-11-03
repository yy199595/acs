#pragma once
#include "HttpLocalRequest.h"
#include <Network/Http/Content/HttpReadContent.h>
namespace GameKeeper
{
	class HttpClientComponent;
	class HttpLolcalGetRequest : public HttpLocalRequest
	{
	public:
		explicit HttpLolcalGetRequest(HttpClientComponent * component);
		~HttpLolcalGetRequest() override = default;
	public:
		XCode Get(const std::string & url, HttpReadContent & response);
	protected:
		bool WriterToBuffer(std::ostream & os) override;
        void OnReceiveBody(asio::streambuf & buf) override;
	private:
        HttpReadContent * mResponse;
	};
}