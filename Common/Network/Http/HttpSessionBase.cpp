#include "HttpSessionBase.h"
#include <Define/CommonDef.h>
#include <istream>
namespace Sentry
{
    HttpSessionBase::HttpSessionBase(ISocketHandler *handler)
            : SessionBase(handler)
    {
        this->mCount = 0;
    }

    void HttpSessionBase::StartReceive()
    {
        if(!this->mSocket->is_open())
        {
            return;
        }
        asio::async_read(this->GetSocket(), this->mStreamBuf, asio::transfer_at_least(1),
                         std::bind(&HttpSessionBase::ReadCallback, this, args1, args2));
//        asio::async_read_until(this->GetSocket(), this->mStreamBuf, "\n",
//                               std::bind(&HttpSessionBase::ReadCallback, this, args1, args2));
    }

    void HttpSessionBase::ReadCallback(const asio::error_code &err, size_t size)
    {
//        std::istream is(&this->mStreamBuf);
//        std::istreambuf_iterator<char> eos;
//        SayNoDebugInfo(std::string(std::istreambuf_iterator<char>(is), eos));
//
//        SayNoDebugInfo(size);
        if(!this->OnReceive(this->mStreamBuf, err))
        {
            asio::error_code err;
            this->mSocket->close(err);
            return;
        }
        this->GetContext().post(std::bind(&HttpSessionBase::StartReceive, this));
    }
}
