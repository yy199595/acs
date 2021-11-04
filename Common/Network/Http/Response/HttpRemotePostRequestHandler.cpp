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
        NetWorkThread *netWorkThread = this->mHttpComponent->GetNetThread();
        netWorkThread->AddTask(&HttpRemoteSession::StartReceiveBody, this->mHttpSession);
        App::Get().GetCorComponent()->YieldReturn(this->mCorId);
    }
}

