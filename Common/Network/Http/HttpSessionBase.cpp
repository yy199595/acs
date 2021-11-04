#include "HttpSessionBase.h"
#include <Define/CommonDef.h>
#include <istream>
#include <Component/IComponent.h>
#include <Network/Http/HttpHandlerBase.h>
namespace GameKeeper
{
    HttpSessionBase::HttpSessionBase(HttpClientComponent * component)
            : mHttpComponent(component)
    {
        this->mCount = 0;	
        this->mIsReadBody = false;
		this->mSocketProxy = nullptr;
    }

    void HttpSessionBase::StartSendHttpMessage()
    {
		AsioTcpSocket & socket = this->mSocketProxy->GetSocket();
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
		GKAssertRet_F(nThread.IsCurrentThread());
		
        if (!socket.is_open())
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
        asio::async_write(socket, this->mStreamBuf, [this, isDone](
                const asio::error_code &err, size_t size)
        {
            if (err)
            {
                this->OnWriteAfter(XCode::HttpNetWorkError);
                return;
            }
            if (!isDone)
            {
				AsioContext & context = this->mSocketProxy->GetContext();
				context.post(std::bind(&HttpSessionBase::StartSendHttpMessage, this));
                return;
            }
            this->OnWriteAfter(XCode::Successful);
        });
    }

    void HttpSessionBase::StartReceiveBody()
    {
		AsioTcpSocket & socket = this->mSocketProxy->GetSocket();
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
		GKAssertRet_F(this->mSocketProxy && this->mSocketProxy->IsOpen());
		if (nThread.IsCurrentThread())
		{
			this->ReceiveBody();
			return;
		}
		nThread.AddTask(&HttpSessionBase::ReceiveBody, this);      
    }

    void HttpSessionBase::StartReceiveHeard()
    {
		AsioTcpSocket & socket = this->mSocketProxy->GetSocket();
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
		GKAssertRet_F(this->mSocketProxy && this->mSocketProxy->IsOpen());
		if (nThread.IsCurrentThread())
		{
			this->ReceiveHeard();
			return;
		}
		nThread.AddTask(&HttpSessionBase::ReceiveHeard, this);

      
    }

	void HttpSessionBase::ReceiveBody()
	{
		AsioTcpSocket & socket = this->mSocketProxy->GetSocket();
		if (!socket.is_open() || !this->mIsReadBody)
		{
			this->OnReceiveBodyAfter(XCode::HttpNetWorkError);
			return;
		}
		asio::async_read(socket, this->mStreamBuf, asio::transfer_at_least(1),
			std::bind(&HttpSessionBase::ReadBodyCallback, this, args1, args2));
	}

	void HttpSessionBase::ReceiveHeard()
	{
		AsioTcpSocket & socket = this->mSocketProxy->GetSocket();
		if (!socket.is_open() || this->mIsReadBody)
		{
			GKDebugFatal("logic error");
			return;
		}
		asio::async_read(socket, this->mStreamBuf, asio::transfer_at_least(1),
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
					AsioContext & context = this->mSocketProxy->GetContext();
					context.post(std::bind(&HttpSessionBase::StartReceiveHeard, this));
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
					AsioContext & context = this->mSocketProxy->GetContext();
					context.post(std::bind(&HttpSessionBase::StartReceiveBody, this));
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
			AsioContext & context = this->mSocketProxy->GetContext();
			context.post(std::bind(&HttpSessionBase::StartReceiveBody, this));
        }
    }
}
