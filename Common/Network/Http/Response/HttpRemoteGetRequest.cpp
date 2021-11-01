//
// Created by zmhy0073 on 2021/11/1.
//
#include "HttpRemoteGetRequest.h"
namespace GameKeeper
{
    bool HttpRemoteGetRequest::OnReceiveBody(asio::streambuf &buf)
    {
		return true;
    }

    bool HttpRemoteGetRequest::WriterToBuffer(std::ostream &os)
    {
		return true;
    }
}