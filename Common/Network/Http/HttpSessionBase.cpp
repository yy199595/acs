#include "HttpSessionBase.h"
#include <Define/CommonLogDef.h>
#include <istream>
#include"Core/App.h"
#include"Util/TimeHelper.h"
#include"Http/HttpHandlerBase.h"
namespace GameKeeper
{
    HttpSessionBase::HttpSessionBase()
    {
        this->mCount = 0;
        this->mIsReadBody = false;
		this->mSocketProxy = nullptr;
    }

    HttpSessionBase::~HttpSessionBase()
    {		
		delete this->mSocketProxy;
    }

    void HttpSessionBase::StartSendHttpMessage()
    {
        NetWorkThread & nThread = this->mSocketProxy->GetThread();
        if(nThread.IsCurrentThread())
        {
            this->SendHttpMessage();
            return;
        }
        nThread.Invoke(&HttpSessionBase::SendHttpMessage, this);
    }

    void HttpSessionBase::SendHttpMessage()
    {
		AsioTcpSocket & socket = this->mSocketProxy->GetSocket();

        if (!socket.is_open())
        {
            this->OnWriterAfter(XCode::NetSendFailure);
            return;
        }
        std::ostream os(&this->mStreamBuf);
        this->WriterToBuffer(os);
        if (this->mStreamBuf.size() == 0)
        {
            return;
        }
        asio::async_write(socket, this->mStreamBuf, [this](
                const asio::error_code &err, size_t size)
        {
            if (err)
            {
				this->OnWriterAfter(XCode::NetSendFailure);
                return;
            }
			this->OnWriterAfter(XCode::Successful);
        });
    }

    void HttpSessionBase::StartReceiveHead()
    {
		AsioTcpSocket & socket = this->mSocketProxy->GetSocket();
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
		
		if (nThread.IsCurrentThread())
		{
			this->ReceiveHead();
			return;
		}
        nThread.Invoke(&HttpSessionBase::ReceiveHead, this);
    }

	void HttpSessionBase::ReceiveHead()
    {
        asio::error_code code;
        AsioTcpSocket &socket = this->mSocketProxy->GetSocket();
        asio::async_read(socket, this->mStreamBuf, asio::transfer_at_least(1),
                         std::bind(&HttpSessionBase::OnRecvHead, this, args1, args2));
    }

    void HttpSessionBase::ReceiveBody()
    {
        asio::error_code code;
        AsioTcpSocket &socket = this->mSocketProxy->GetSocket();
        asio::async_read(socket, this->mStreamBuf, asio::transfer_at_least(1),
                         std::bind(&HttpSessionBase::OnRecvBody, this, args1, args2));
    }

    void HttpSessionBase::OnRecvHead(const asio::error_code &err, size_t size)
    {
        if(err == asio::error::eof)
        {
            this->OnComplete(XCode::Successful);
        }
        else if(err)
        {
            this->OnComplete(XCode::NetReceiveFailure);
        }
        else
        {
            if (!this->mIsReadBody)
            {
                const char *data = asio::buffer_cast<const char *>(this->mStreamBuf.data());
                const char *pos = strstr(data, "\r\n\r\n");
                if (pos == nullptr)
                {
                    if(this->mStreamBuf.size() >= HttpHeadMaxCount)
                    {
                        this->OnComplete(XCode::NetBigDataShutdown);
                        return;
                    }
					AsioContext & context = this->mSocketProxy->GetContext();
					context.post(std::bind(&HttpSessionBase::ReceiveHead, this));
                    return;
                }
                this->mIsReadBody = true;
                if(this->OnReceiveHead(mStreamBuf))
                {
                    if(this->mStreamBuf.size() > 0)
                    {
                        if(!this->OnReceiveBody(this->mStreamBuf))
                        {
                            this->OnComplete(XCode::Successful);
                            return;
                        }
                    }
                    this->ReceiveBody();
                }
            }
        }
    }

    void HttpSessionBase::OnRecvBody(const asio::error_code &err, size_t size)
    {
        if(err)
        {
            this->OnComplete(XCode::HttpNetWorkError);
            return;
        }
        if(this->OnReceiveBody(this->mStreamBuf))
        {
            this->ReceiveBody();
            return;
        }
        this->OnComplete(XCode::Successful);
    }
}
