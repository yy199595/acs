#include "HttpRequest.h"
#include<Core/App.h>
#include<Define/CommonDef.h>
#include <Util/StringHelper.h>
#include<Network/Http/HttpLocalSession.h>
namespace Sentry
{
    HttpRequest::HttpRequest(ISocketHandler *handler, const std::string &name)
            : mHttpSession(nullptr), mName(name),mMainTaskPool(App::Get().GetTaskScheduler())
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
                this->OnReceiveHeard(std::string(data, size));
            }
        }
        if (this->mIsEnd && strem.size() > 0)
        {
            this->OnReceiveBody(strem);
        }
        return true;
    }

    bool HttpRequest::ConnectRemote()
    {
        if (this->mHttpSession == nullptr)
        {
            return false;
        }
        this->mHttpSession->StartConnectHost(this->mName, this->mHost, this->mPort);
        return true;
    }
}


