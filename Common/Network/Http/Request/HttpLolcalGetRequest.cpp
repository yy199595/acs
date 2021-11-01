#include "HttpLolcalGetRequest.h"
#include <Network/NetworkHelper.h>
namespace GameKeeper
{
	HttpLolcalGetRequest::HttpLolcalGetRequest(HttpClientComponent * component)
		:HttpLocalRequest(component)
	{
		this->mCorId = 0;
		this->mResponse = nullptr;
	}

	XCode HttpLolcalGetRequest::Get(const std::string & url, std::string & response)
	{
		this->mResponse = &response;
        return this->StartHttpRequest(url);
	}

	bool HttpLolcalGetRequest::WriterToBuffer(std::ostream & os)
	{
		os << "GET " << mPath << " HTTP/1.0\r\n";
		os << "Host: " << mHost << "\r\n";
		os << "Accept: */*\r\n";
		os << "Connection: close\r\n\r\n";
		return true;
	}

	bool HttpLolcalGetRequest::OnReceiveBody(asio::streambuf & buf)
	{
        std::istream is(&buf);
        while(buf.size() > 0)
        {
            size_t size = is.readsome(this->mHandlerBuffer, 1024);
            this->mResponse->append(this->mHandlerBuffer, size);
        }
        return true;
	}
}