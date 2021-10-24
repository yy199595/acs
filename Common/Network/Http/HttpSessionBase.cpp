#include "HttpSessionBase.h"
#include <Define/CommonDef.h>
namespace Sentry
{
	HttpSessionBase::HttpSessionBase(ISocketHandler * handler)
		: SessionBase(handler)
	{

	}

	void HttpSessionBase::StartReceive()
	{
		asio::async_read(this->GetSocket(), this->mStreamBuf, asio::transfer_at_least(1), 
			std::bind(&HttpSessionBase::ReadCallback, this, args1, args2));
	}

	void HttpSessionBase::ReadCallback(const asio::error_code & err, size_t size)
	{
		if (err)
		{
			this->OnClose();
		}
		else
		{
			this->GetContext().post(std::bind(&HttpSessionBase::StartReceive, this));
		}
		this->OnReceive(mStreamBuf, err);
	}
}
