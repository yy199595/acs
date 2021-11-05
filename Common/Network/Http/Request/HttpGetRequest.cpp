#include "HttpGetRequest.h"
#include <Network/NetworkHelper.h>
#include<Define/CommonDef.h>
namespace GameKeeper
{
	HttpGetRequest::HttpGetRequest(HttpClientComponent * component)
		:HttpRequest(component)
	{
		this->mCorId = 0;
#ifdef __DEBUG__
		this->mCurrentLength = 0;
#endif // __DEBUG__	
		this->mResponse = nullptr;
	}

	XCode HttpGetRequest::Get(const std::string & url, HttpReadContent & response)
	{
		this->mResponse = &response;
        return this->StartHttpRequest(url);
	}

	bool HttpGetRequest::WriterToBuffer(std::ostream & os)
	{
		os << "GET " << mPath << " HTTP/1.0\r\n";
		os << "Host: " << mHost << "\r\n";
		os << "Accept: */*\r\n";
		os << "Connection: close\r\n\r\n";
		return true;
	}

	void HttpGetRequest::OnReceiveBody(asio::streambuf & buf)
    {
        std::istream is(&buf);
        while (buf.size() > 0)
        {
            size_t size = is.readsome(this->mHandlerBuffer, 1024);
            this->mResponse->OnReadContent(this->mHandlerBuffer, size);
#ifdef __DEBUG__
            auto fileContent = dynamic_cast<HttpReadFileContent *>(this->mResponse);
            if (fileContent != nullptr)
            {
                size_t dataLength = this->GetContentLength();
                if (dataLength > 0)
                {
                    this->mCurrentLength += size;
                    size_t process = this->mCurrentLength * 1000 / dataLength;
                    GKDebugInfo("download file " << fileContent->GetPaht() << " [" << process / 10.0f << "%]");
                }
            }
#endif // __DEBUG__
        }
    }
}