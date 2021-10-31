#include "HttpSessionBase.h"
#include <Define/CommonDef.h>
#include <istream>
#include <Network/Http/HttpHandlerBase.h>
namespace Sentry
{
	HttpSessionBase::HttpSessionBase(ISocketHandler *  handler)
		: SessionBase(handler)
	{
		this->mCount = 0;
		this->mIsReadBody = false;
	}

	void HttpSessionBase::StartSendHttpMessage()
	{
		if (!this->mSocket->is_open())
		{
			return;
		}
		std::ostream os(&this->mStreamBuf);
		bool isDone = this->WriterToBuffer(os);
		asio::async_write(this->GetSocket(), this->mStreamBuf, [this, isDone](
			const asio::error_code & err, size_t size)
		{
			if (err)
			{
				asio::error_code code;
				this->GetSocket().close(code);
			}
			else if (!isDone)
			{
				this->GetContext().post(std::bind(&HttpSessionBase::StartSendHttpMessage, this));
			}
		});
	}

	void HttpSessionBase::StartReceive()
    {
        if(!this->mSocket->is_open())
        {
            return;
        }
        asio::async_read(this->GetSocket(), this->mStreamBuf, asio::transfer_at_least(1),
                         std::bind(&HttpSessionBase::ReadCallback, this, args1, args2));
    }

	void HttpSessionBase::ReadCallback(const asio::error_code &err, size_t size)
    {
		if (!this->mIsReadBody)
		{
			const char *data = asio::buffer_cast<const char *>(this->mStreamBuf.data());
			const char *pos = strstr(data, "\r\n\r\n");
			if (pos == nullptr)
			{
				this->GetContext().post(std::bind(&HttpSessionBase::StartReceive, this));
				return;
			}
			size_t size = pos - data + strlen("\r\n\r\n");
			if (size != 0)
			{
				this->mIsReadBody = true;
				if (!this->OnReceiveHeard(mStreamBuf, err))
				{
					asio::error_code code;
					this->mSocket->close(code);
					return;
				}
			}
		}
		if (this->mIsReadBody)
		{
			if (!this->OnReceiveBody(mStreamBuf, err))
			{
				asio::error_code code;
				this->mSocket->close(code);
				return;
			}
		}
        this->GetContext().post(std::bind(&HttpSessionBase::StartReceive, this));
    }
}
