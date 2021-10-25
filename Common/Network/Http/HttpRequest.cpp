#include "HttpRequest.h"
#include<Core/App.h>
#include<Define/CommonDef.h>
#include <Util/StringHelper.h>
#include<Network/Http/HttpLocalSession.h>
namespace Sentry
{
    HttpRequest::HttpRequest(ISocketHandler *handler, const std::string &name)
            : mHttpSession(nullptr), mName(name)
    {
        this->mHttpCode = 0;
        this->mReadCount = 0;
        this->mIsEnd = false;
        this->mHttpSession = new HttpLocalSession(handler, *this);
    }

    HttpRequest::~HttpRequest() noexcept
    {
        if(this->mHttpSession)
        {
            delete this->mHttpSession;
        }
    }

    bool HttpRequest::OnReceive(asio::streambuf &strem, const asio::error_code &err)
    {
        if (err == asio::error::eof)
        {
            this->OnReceiveDone(false);
            return false;
        }
        else if (err)
        {
            this->OnReceiveDone(true);
            return false;
        }
        if (this->mReadCount == 0)
        {
            std::istream is(&strem);
            is >> this->mVersion >> this->mHttpCode >> this->mError;
            SayNoDebugWarning(this->mVersion << "  " << this->mHttpCode << "  " << this->mError);
        }

        this->mReadCount++;
        if (!this->mIsEnd && strem.size() > 0)
        {
            std::istream is(&strem);
            const char *data = asio::buffer_cast<const char *>(strem.data());
            const char *pos = strstr(data, "\r\n\r\n");
            if(pos == nullptr)
            {
                return true;
            }
            size_t size = pos - data + strlen("\r\n\r\n");
            if (size != 0)
            {
                is.ignore(size);
                this->mIsEnd = true;
                this->mHeard.append(data, size);
            }
        }
        if (this->mIsEnd && strem.size() > 0)
        {
            this->OnReceiveBody(strem);
        }
        return true;
    }

    bool HttpRequest::Connect(const std::string &host, unsigned short port)
    {
        if (this->mHttpSession == nullptr)
        {
            return false;
        }
        this->mHost = host;
        this->mPort = port;
        this->mHttpSession->StartConnectHost(this->mName, host, port);
        return true;
    }
}

namespace Sentry
{

    HttpGetRequest::HttpGetRequest(ISocketHandler * handler)
            : HttpRequest(handler, "HttpGetRequest"),
              mMainTaskPool(App::Get().GetTaskScheduler())
    {
        this->mCorComponent = App::Get().GetCoroutineComponent();
    }

	void HttpGetRequest::GetRquestParame(asio::streambuf & strem)
	{
		std::ostream os(&strem);
		os << "GET " << mPath << " HTTP/1.0\r\n";
		os << "Host: " << mHost << "\r\n";
		os << "Accept: */*\r\n";
		os << "Connection: close\r\n\r\n";
	}

    void HttpGetRequest::OnReceiveDone(bool hasError)
    {
        this->mCode = hasError ? XCode::HttpNetWorkError : XCode::Successful;
        this->mMainTaskPool.AddMainTask(&CoroutineComponent::Resume, this->mCorComponent, this->mCoroutineId);
    }

    void HttpGetRequest::OnReceiveBody(asio::streambuf &strem)
    {
        std::istream is(&strem);
        std::istreambuf_iterator<char> eos;
        SayNoDebugWarning(strem.size());
        this->mData.append(std::istreambuf_iterator<char>(is), eos);
    }


	XCode HttpGetRequest::Get(const std::string & url, std::string & response)
	{
		if (!NetworkHelper::ParseHttpUrl(url, this->mHost, this->mPort, this->mPath))
		{
			return XCode::HttpUrlParseError;
		}
		if (!this->Connect(this->mHost, this->mPort))
		{
			return XCode::HttpNetWorkError;
		}
		this->mCorComponent->YieldReturn(this->mCoroutineId);
		if (this->mCode != XCode::Successful)
		{
			return this->mCode;
		}
        if(this->GetHttpCode() != 200)
        {
            return XCode::HttpResponseError;
        }
        response = this->mData;
		return XCode::Successful;
	}
	
}
