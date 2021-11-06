//
// Created by zmhy0073 on 2021/10/26.
//

#include "HttpRemoteSession.h"
#include <Core/App.h>
#include <Scene/ProtocolComponent.h>
#include <Network/Http/HttpClientComponent.h>
#include <Network/Http/Response/HttpGettHandler.h>
#include <Network/Http/Response/HttpPostHandler.h>
namespace GameKeeper
{
    HttpRemoteSession::HttpRemoteSession(HttpClientComponent *component)
        : HttpSessionBase(component)
    {
        this->mHttpHandler = nullptr;
        this->mHttpComponent = component;
    }

	void HttpRemoteSession::SetSocketProxy(SocketProxy * scoketProxy)
    {
        delete this->mSocketProxy;
        this->mSocketProxy = scoketProxy;
        this->StartReceiveHeard();
    }

	HttpHandlerBase * HttpRemoteSession::GetHandler()
	{
		return this->mHttpHandler;
	}

	bool HttpRemoteSession::OnReceiveHeard(asio::streambuf & streamBuf)
	{
		std::istream is(&streamBuf);
		is >> this->mMethod;
		if (this->mMethod == "GET")
		{
			this->mHttpHandler = new HttpGettHandler(this->mHttpComponent, this);
		}
		else if (this->mMethod == "POST")
		{
			this->mHttpHandler = new HttpPostHandler(this->mHttpComponent, this);
		}
		if (this->mHttpHandler == nullptr)
		{
			return false;
		}
		return this->mHttpHandler->OnReceiveHeard(streamBuf);
	}
}