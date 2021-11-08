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

	bool HttpRemoteSession::OnReceiveHeard(asio::streambuf & streamBuf)
    {
        std::istream is(&streamBuf);
        is >> this->mMethod;
        this->mHttpHandler = nullptr;
        auto iter = this->mHandlerMap.find(this->mMethod);
        if (iter == this->mHandlerMap.end())
        {
            GKDebugError("not find http method " << this->mMethod);
            return false;
        }
        this->mHttpHandler = iter->second;
        return this->mHttpHandler->OnReceiveHeard(streamBuf);
    }

    void HttpRemoteSession::OnReceiveHeardAfter(XCode code)
    {
        this->mCode = code;
        if(code == XCode::Successful && this->mHttpHandler == nullptr)
        {
            this->mCode = XCode::HttpMethodNotFound;
        }
        //通知到主线程处理
		CoroutineComponent * corComponent = App::Get().GetCorComponent();
		MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();
		taskScheduler.AddMainTask(&HttpClientComponent::OnRequest, this->mHttpComponent, this);
    }

    size_t HttpRemoteSession::ReadFromStream(char *buffer, size_t count)
    {
        if(this->mHttpHandler == nullptr)
        {
            return 0;
        }
        size_t size = this->mHttpHandler->ReadFromStream(buffer, count);
        if(size != 0)
        {
            return size;
        }
        if (this->mStreamBuf.size() > 0)
        {
            std::istream is(&this->mStreamBuf);
            return is.readsome(buffer, count);
        }

        if (!this->mSocketProxy->IsOpen())
        {
            return 0;
        }
        NetWorkThread &netWorkThread = this->mSocketProxy->GetThread();
        netWorkThread.AddTask(&HttpRemoteSession::StartReceiveBody, this);
        App::Get().GetCorComponent()->YieldReturn(this->mCorId);

        if(this->mStreamBuf.size() > 0)
        {
            std::istream is(&this->mStreamBuf);
            return is.readsome(buffer, count);
        }
        return 0;
    }

    void HttpRemoteSession::StartReceiveBody()
    {
        asio::error_code code;   
		GKAssertRet_F(this->mSocketProxy->IsOpen());
		AsioTcpSocket &socket = this->mSocketProxy->GetSocket();
        if (socket.available(code) == 0)
        {
            auto corComponent = App::Get().GetCorComponent();
            NetWorkThread &netWorkThread = this->mSocketProxy->GetThread();
            netWorkThread.AddTask(&CoroutineComponent::Resume, corComponent, this->mCorId);
        }
        else
        {
            asio::async_read(socket, this->mStreamBuf, asio::transfer_at_least(1),
                             std::bind(&HttpRemoteSession::ReadBodyCallback, this, args1, args2));
        }
    }

    void HttpRemoteSession::ReadBodyCallback(const asio::error_code &err, size_t size)
    {
        if(err)
        {
            this->mCode = XCode::NetReceiveFailure;
        }
        auto corComponent = App::Get().GetCorComponent();
        NetWorkThread &netWorkThread = this->mSocketProxy->GetThread();
        netWorkThread.AddTask(&CoroutineComponent::Resume, corComponent, this->mCorId);
    }


    void HttpRemoteSession::OnWriterAfter(XCode code)
    {

    }

}