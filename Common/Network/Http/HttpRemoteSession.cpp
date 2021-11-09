//
// Created by zmhy0073 on 2021/10/26.
//

#include "HttpRemoteSession.h"
#include <Core/App.h>
#include <Scene/ProtocolComponent.h>
#include <Network/Http/HttpComponent.h>
#include <Network/Http/Response/HttpGettHandler.h>
#include <Network/Http/Response/HttpPostHandler.h>
#include <Method/HttpServiceMethod.h>
namespace GameKeeper
{
    HttpRemoteSession::HttpRemoteSession(HttpComponent *component)
        : HttpSessionBase(component)
    {
        this->mReadSize = 0;
        this->mHttpHandler = nullptr;
        this->mSocketProxy = nullptr;
        this->mHttpComponent = component;
        this->mHandlerMap["GET"] = new HttpGettHandler(component);
        this->mHandlerMap["POST"] = new HttpPostHandler(component);
    }

    HttpRemoteSession::~HttpRemoteSession() noexcept
    {
        auto iter = this->mHandlerMap.begin();
        for(; iter != this->mHandlerMap.end(); iter++)
        {
            delete iter->second;
        }
        this->mHandlerMap.clear();
    }

    void HttpRemoteSession::Clear()
    {
        HttpSessionBase::Clear();

        delete this->mSocketProxy;
        this->mSocketProxy = nullptr;
        this->mHttpHandler = nullptr;
        this->mMethod.clear();
        this->mAddress.clear();
    }

    void HttpRemoteSession::Start(SocketProxy *socketProxy)
    {
        delete this->mSocketProxy;
        this->mSocketProxy = socketProxy;
        this->StartReceiveHeard();
    }

	HttpHandlerBase * HttpRemoteSession::GetHandler()
	{
		return this->mHttpHandler;
	}

	void HttpRemoteSession::OnReceiveHeard(asio::streambuf & streamBuf)
    {
        std::istream is(&streamBuf);
        is >> this->mMethod;
        this->mHttpHandler = nullptr;
        auto iter = this->mHandlerMap.find(this->mMethod);
        if (iter == this->mHandlerMap.end())
        {
            GKDebugError("not find http method " << this->mMethod);
            return;
        }
        this->mHttpHandler = iter->second;
        if(!this->mHttpHandler->OnReceiveHeard(streamBuf))
        {
            this->mHttpHandler->SetResponseCode(HttpStatus::BAD_REQUEST);
            this->StartSendHttpMessage();
        }
        else
        {
            const std::string & method = this->mHttpHandler->GetMethod();
            const std::string & service = this->mHttpHandler->GetComponent();
            if(this->mHttpComponent->GetHttpMethod(service, method) == nullptr)
            {
                this->mHttpHandler->SetResponseCode(HttpStatus::NOT_FOUND);
                this->StartSendHttpMessage();
            }
        }
    }

    void HttpRemoteSession::SetCode(XCode code)
    {
        this->mCode = code;
        CoroutineComponent * corComponent = App::Get().GetCorComponent();
        MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.AddMainTask(&HttpComponent::OnRequest, this->mHttpComponent, this);
    }

    void HttpRemoteSession::OnReceiveHeardAfter(XCode code)
    {
        if(code != XCode::Successful || this->mHttpHandler == nullptr)
        {
            this->SetCode(code);
        }
        else if(this->mHttpHandler->GetType() == HttpMethodType::GET)
        {
            this->SetCode(XCode::Successful);
        }
        else
        {
            this->StartReceiveBody();
        }
    }

    void HttpRemoteSession::StartReceiveBody()
    {
        asio::error_code code;   
		GKAssertRet_F(this->mSocketProxy->IsOpen());
		AsioTcpSocket &socket = this->mSocketProxy->GetSocket();
        if (socket.available(code) == 0)
        {
            this->SetCode(XCode::Successful);
        }
        else
        {
            asio::async_read(socket, this->mStreamBuf, asio::transfer_at_least(1),
                             std::bind(&HttpRemoteSession::ReadBodyCallback, this, args1, args2));
        }
    }

    void HttpRemoteSession::ReadBodyCallback(const asio::error_code &err, size_t size)
    {
        if(err == asio::error::eof)
        {
            this->SetCode(XCode::Successful);
        }
        else if(err)
        {
            this->SetCode(XCode::NetReceiveFailure);
        }
        else
        {
            this->mHttpHandler->OnReceiveBody(this->mStreamBuf);
            AsioContext  & context = this->mSocketProxy->GetContext();
            context.post(std::bind(&HttpRemoteSession::StartReceiveBody, this));
        }
    }

    void HttpRemoteSession::OnWriterAfter(XCode code)
    {

    }
}