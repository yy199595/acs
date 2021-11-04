//
// Created by zmhy0073 on 2021/11/4.
//

#include "HttpRemotePostRequestHandler.h"
#include <Core/App.h>
#include <Network/Http/HttpRemoteSession.h>
#include <Network/Http/HttpClientComponent.h>
namespace GameKeeper
{
    HttpRemotePostRequestHandler::HttpRemotePostRequestHandler(HttpClientComponent *component, HttpRemoteSession *session)
                                                               : HttpRemoteRequestHandler(component, session)
    {
        this->mCorId = 0;
        this->mContent = nullptr;
    }

    XCode HttpRemotePostRequestHandler::GetContent(HttpReadContent &content)
    {
        this->mContent = &content;
        this->mHttpSession->StartReceiveBody();
		NetWorkThread & netWorkThread = this->GetSession()->GetThread();
        netWorkThread.AddTask(&HttpRemoteSession::StartReceiveBody, this->mHttpSession);
        App::Get().GetCorComponent()->YieldReturn(this->mCorId);
        return this->GetErrorCode();
    }

    void HttpRemotePostRequestHandler::OnReceiveBodyAfter(XCode code)
    {
        this->mCode = code;
        MainTaskScheduler & taskScheduler =  App::Get().GetTaskScheduler();
        CoroutineComponent * coroutineComponent = App::Get().GetCorComponent();
        taskScheduler.AddMainTask(&CoroutineComponent::Resume, coroutineComponent, this->mCorId);
    }

    void HttpRemotePostRequestHandler::OnReceiveBody(asio::streambuf &buf)
    {
        std::istream is(&buf);
        while(buf.size() > 0)
        {
            size_t size = is.readsome(this->mHandlerBuffer, 1024);
            this->mContent->OnReadContent(this->mHandlerBuffer, size);
        }
    }
}

