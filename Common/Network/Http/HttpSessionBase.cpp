#include "HttpSessionBase.h"
#include <Define/CommonDef.h>
#include <istream>
#include <Component/IComponent.h>
#include <Network/Http/HttpHandlerBase.h>
namespace GameKeeper
{
    HttpSessionBase::HttpSessionBase(ISocketHandler *handler, const std::string & name)
            : SessionBase(handler, name)
    {
        this->mCount = 0;
        this->mIsReadBody = false;
    }

    void HttpSessionBase::StartSendHttpMessage()
    {
#ifdef __DEBUG__
        if (!this->mHandler.GetNetThread()->IsCurrentThread())
        {
            GKDebugFatal("use in not http thread");
            return;
        }
#endif
        if (!this->mSocket->is_open())
        {
            this->OnWriteAfter(XCode::HttpNetWorkError);
            return;
        }
        std::ostream os(&this->mStreamBuf);
        bool isDone = this->WriterToBuffer(os);
        if (this->mStreamBuf.size() == 0)
        {
            return;
        }
        asio::async_write(this->GetSocket(), this->mStreamBuf, [this, isDone](
                const asio::error_code &err, size_t size)
        {
            if (err)
            {
                this->OnWriteAfter(XCode::HttpNetWorkError);
                return;
            }
            if (!isDone)
            {
                this->GetContext().post(std::bind(&HttpSessionBase::StartSendHttpMessage, this));
                return;
            }
            this->OnWriteAfter(XCode::Successful);
        });
    }

    void HttpSessionBase::StartReceiveBody()
    {
#ifdef __DEBUG__
        if(!this->mHandler.GetNetThread()->IsCurrentThread())
        {
            GKDebugFatal("use int not http thread");
            return;
        }
#endif
        if(!this->mSocket->is_open() || !this->mIsReadBody)
        {
            this->OnReceiveBodyAfter(XCode::HttpNetWorkError);
            return;
        }
        asio::async_read(this->GetSocket(), this->mStreamBuf, asio::transfer_at_least(1),
                         std::bind(&HttpSessionBase::ReadBodyCallback, this, args1, args2));
    }

    void HttpSessionBase::StartReceiveHeard()
    {
#ifdef __DEBUG__
        if(!this->mHandler.GetNetThread()->IsCurrentThread())
        {
            GKDebugFatal("use int not http thread");
            return;
        }
#endif
        if (!this->mSocket->is_open() || this->mIsReadBody)
        {
            GKDebugFatal("logic error");
            return;
        }
        asio::async_read(this->GetSocket(), this->mStreamBuf, asio::transfer_at_least(1),
                         std::bind(&HttpSessionBase::ReadHeardCallback, this, args1, args2));
    }

    void HttpSessionBase::ReadHeardCallback(const asio::error_code &err, size_t size)
    {
        if(err == asio::error::eof)
        {
            this->OnReceiveBodyAfter(XCode::Successful);
        }
        else if(err)
        {
            this->OnReceiveBodyAfter(XCode::HttpNetWorkError);
        }
        else
        {
            if (!this->mIsReadBody)
            {
                const char *data = asio::buffer_cast<const char *>(this->mStreamBuf.data());
                const char *pos = strstr(data, "\r\n\r\n");
                if (pos == nullptr)
                {
                    this->GetContext().post(std::bind(&HttpSessionBase::StartReceiveHeard, this));
                    return;
                }

                size_t pos1 = pos - data + strlen("\r\n\r\n");
                if (this->OnReceiveHeard(mStreamBuf, pos1))
                {
                    this->mIsReadBody = true;
                    if (this->mStreamBuf.size() > 0)
                    {
                        this->OnReceiveBody(this->mStreamBuf);
                    }
                    this->GetContext().post(std::bind(&HttpSessionBase::StartReceiveBody, this));
                }
                this->OnReceiveBodyAfter(XCode::Successful);
                this->mIsReadBody = true;
            }
        }
    }

    void HttpSessionBase::ReadBodyCallback(const asio::error_code &err, size_t size)
    {
        if(err == asio::error::eof)
        {
            this->OnReceiveBodyAfter(XCode::Successful);
        }
        else if(err)
        {
            this->OnReceiveBodyAfter(XCode::HttpNetWorkError);
        }
        else
        {
            this->OnReceiveBody(this->mStreamBuf);
            this->GetContext().post(std::bind(&HttpSessionBase::StartReceiveBody, this));
        }
    }
}
