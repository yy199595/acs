#include"HttpGetRequest.h"
#include<Define/CommonDef.h>
#include<NetworkHelper.h>
#include<Http/Content/HttpReadContent.h>
namespace GameKeeper
{
	HttpGetRequest::HttpGetRequest(HttpComponent * component)
		:HttpRequest(component)
	{
#ifdef __DEBUG__
		this->mCurrentLength = 0;
#endif // __DEBUG__	
		this->mResponse = nullptr;
	}

    bool HttpGetRequest::Init(const std::string &url, HttpReadContent &response)
    {
        if(!this->ParseUrl(url))
        {
            return false;
        }
        this->mResponse = &response;
        return true;
    }

	void HttpGetRequest::WriteHead(std::ostream &os)
	{
		os << "GET " << this->GetPath() << " " << HttpVersion << "\r\n";
		os << "Host: " << this->GetHost() << "\r\n";
		os << "Accept: */*\r\n";
		os << "Connection: close\r\n\r\n";
	}

    bool HttpGetRequest::WriteBody(std::ostream &os)
    {
        return true;
    }

    void HttpGetRequest::Clear()
    {
        HttpRequest::Clear();
        this->mResponse = nullptr;
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