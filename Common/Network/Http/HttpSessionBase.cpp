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
            this->OnWriterAfter(XCode::NetSendFailure);
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
				this->OnWriterAfter(XCode::NetSendFailure);
                return;
            }
            if (!isDone)
            {
				AsioContext & context = this->mSocketProxy->GetContext();
				context.post(std::bind(&HttpSessionBase::StartSendHttpMessage, this));
                return;
            }
			this->OnWriterAfter(XCode::Successful);
        });
    }
    
    void HttpSessionBase::StartReceiveHeard()
    {
		/*GKAssertRet_F(this->mSocketProxy);
		GKAssertRet_F(this->mSocketProxy->IsOpen());*/
		AsioTcpSocket & socket = this->mSocketProxy->GetSocket();
		NetWorkThread & nThread = this->mSocketProxy->GetThread();
		
		if (nThread.IsCurrentThread())
		{
			this->ReceiveHeard();
			return;
		}
		nThread.AddTask(&HttpSessionBase::ReceiveHeard, this);
    }

	void HttpSessionBase::ReceiveHeard()
	{
        asio::ip::tcp::iostream io;

        AsioTcpSocket & socket = this->mSocketProxy->GetSocket();
		if (!socket.is_open() || this->mIsReadBody)
		{
			GKDebugFatal("logic error");
			this->OnReceiveHeardAfter(XCode::NetReceiveFailure);
            return;
		}
		
		asio::async_read(socket, this->mStreamBuf, asio::transfer_at_least(1),
			std::bind(&HttpSessionBase::ReadHeardCallback, this, args1, args2));
	}

    void HttpSessionBase::Clear()
    {
        this->mCount = 0;
        this->mAddress.clear();
        this->mIsReadBody = false;
        delete this->mSocketProxy;
        size_t size = this->mStreamBuf.size();
        if(size > 0)
        {
            std::istream(&this->mStreamBuf).ignore(size);
        }
    }

    void HttpSessionBase::ReadHeardCallback(const asio::error_code &err, size_t size)
    {
        if(err == asio::error::eof)
        {
			this->OnReceiveHeardAfter(XCode::Successful);
        }
        else if(err)
        {
			this->OnReceiveHeardAfter(XCode::NetReceiveFailure);
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
                this->mIsReadBody = true;
                this->OnReceiveHeard(mStreamBuf);
				this->OnReceiveHeardAfter(XCode::Successful);
            }
        }
    }
}
