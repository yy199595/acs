#include "HttpSessionBase.h"
#include <Define/CommonDef.h>
#include <istream>
#include <Util/TimeHelper.h>
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

    HttpSessionBase::~HttpSessionBase()
    {		
		delete this->mSocketProxy;
    }

    void HttpSessionBase::StartSendHttpMessage()
    {
		AsioTcpSocket & socket = this->mSocketProxy->GetSocket();
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
		GKAssertRet_F(nThread.IsCurrentThread() && this->GetHandler());
		
        if (!socket.is_open())
        {
			this->GetHandler()->OnWriterAfter(XCode::NetSendFailure);
            return;
        }
        std::ostream os(&this->mStreamBuf);
        bool isDone = this->GetHandler()->WriterToBuffer(os);
        if (this->mStreamBuf.size() == 0)
        {
            return;
        }
        asio::async_write(socket, this->mStreamBuf, [this, isDone](
                const asio::error_code &err, size_t size)
        {
            if (err)
            {
				this->GetHandler()->OnWriterAfter(XCode::NetSendFailure);
                return;
            }
            if (!isDone)
            {
				AsioContext & context = this->mSocketProxy->GetContext();
				context.post(std::bind(&HttpSessionBase::StartSendHttpMessage, this));
                return;
            }
			this->GetHandler()->OnWriterAfter(XCode::Successful);
        });
    }


    void HttpSessionBase::StartReceiveBody()
    {		
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
		GKAssertRet_F(this->GetHandler());
		AsioTcpSocket & socket = this->mSocketProxy->GetSocket();
		if (!socket.is_open() || !this->mIsReadBody)
		{
			this->GetHandler()->OnReceiveBodyAfter(XCode::NetReceiveFailure);
			return;
		}
		asio::error_code code;
		if (socket.available(code) <= 0)
		{
			this->GetHandler()->OnReceiveBodyAfter(XCode::Successful);
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
			this->GetHandler()->OnReceiveHeardAfter(XCode::NetReceiveFailure);
            return;
		}
		
		asio::async_read(socket, this->mStreamBuf, asio::transfer_at_least(1),
			std::bind(&HttpSessionBase::ReadHeardCallback, this, args1, args2));
	}

    void HttpSessionBase::ReadHeardCallback(const asio::error_code &err, size_t size)
    {				
		
        if(err == asio::error::eof)
        {
			GKAssertRet_F(this->GetHandler());
			this->GetHandler()->OnReceiveHeardAfter(XCode::Successful);
        }
        else if(err)
        {
			GKAssertRet_F(this->GetHandler());
			this->GetHandler()->OnReceiveHeardAfter(XCode::NetReceiveFailure);
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
					context.post(std::bind(&HttpSessionBase::ReceiveHeard, this));
                    return;
                }
              
                if (this->OnReceiveHeard(mStreamBuf))
                {
                    this->mIsReadBody = true;
                    if (this->mStreamBuf.size() > 0)
                    {
						GKAssertRet_F(this->GetHandler());
						this->GetHandler()->OnReceiveBody(this->mStreamBuf);
                    }
					AsioContext & context = this->mSocketProxy->GetContext();
					context.post(std::bind(&HttpSessionBase::ReceiveBody, this));
                }
				GKAssertRet_F(this->GetHandler());
				this->GetHandler()->OnReceiveHeardAfter(XCode::Successful);
                this->mIsReadBody = true;
            }
        }
    }

    void HttpSessionBase::ReadBodyCallback(const asio::error_code &err, size_t size)
    {
		GKAssertRet_F(this->GetHandler());
        if(err == asio::error::eof)
        {
			this->GetHandler()->OnReceiveBodyAfter(XCode::Successful);
        }
        else if(err)
        {
			this->GetHandler()->OnReceiveBodyAfter(XCode::NetReceiveFailure);
        }
        else
        {
			this->GetHandler()->OnReceiveBody(this->mStreamBuf);
			AsioContext & context = this->mSocketProxy->GetContext();
			context.post(std::bind(&HttpSessionBase::ReceiveBody, this));
        }
    }
}
