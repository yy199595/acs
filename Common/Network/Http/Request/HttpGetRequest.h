#pragma once
#include"HttpRequest.h"
namespace GameKeeper
{
	class HttpReadContent;
	class HttpClientComponent;
	class HttpGetRequest : public HttpRequest
	{
	public:
		explicit HttpGetRequest(const std::string & url);
		~HttpGetRequest() override = default;
	protected:
        void WriteToSendBuffer(std::ostream &os) final;
	private:
        std::string mResponse;
	};
}