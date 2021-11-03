#include "HttpLolcalGetRequest.h"
#include <Network/NetworkHelper.h>
#include<Define/CommonDef.h>
namespace GameKeeper
{
	HttpLolcalGetRequest::HttpLolcalGetRequest(HttpClientComponent * component)
		:HttpLocalRequest(component)
	{
		this->mCorId = 0;
#ifdef __DEBUG__
		this->mCurrentLength = 0;
#endif // __DEBUG__	
		this->mResponse = nullptr;
	}

	XCode HttpLolcalGetRequest::Get(const std::string & url, HttpReadContent & response)
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

	void HttpLolcalGetRequest::OnReceiveBody(asio::streambuf & buf)
	{
		std::istream is(&buf);
		while (buf.size() > 0)
		{
			size_t size = is.readsome(this->mHandlerBuffer, 1024);
			this->mResponse->OnReadContent(this->mHandlerBuffer, size);
#ifdef __DEBUG__
			/*size_t dataLength = this->GetContentLength();
			if (dataLength > 0)
			{
				this->mCurrentLength += size;
				double sum = dataLength / (1024 * 1024.0f);
				double mb = this->mCurrentLength / (1024 * 1024.0f);
				GKDebugInfo("load data [" << mb << "mb/" << sum << "mb]");
			}*/
#endif // __DEBUG__

		}
	}
}