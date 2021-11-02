//
// Created by zmhy0073 on 2021/11/1.
//

#include "HttpRemoteRequest.h"
#include <Core/App.h>
#include <Network/Http/HttpClientComponent.h>
#include <Network/Http/HttpRemoteSession.h>
namespace GameKeeper
{

    HttpRemoteRequest::HttpRemoteRequest(HttpClientComponent *component, HttpRemoteSession *session)
        : mHttpSession(session), mHttpComponent(component)
    {
        this->mWriteCount = 0;
        this->mHttpContent = nullptr;
    }

    HttpRemoteRequest::~HttpRemoteRequest() noexcept
    {
        delete this->mHttpContent;
    }

    bool HttpRemoteRequest::OnSessionError(const asio::error_code &code)
    {
        if(code == asio::error::eof)
        {
            NoticeMainThread();
        }
        return false;
    }

    void HttpRemoteRequest::NoticeMainThread()
    {
        MainTaskScheduler &taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.AddMainTask(&HttpClientComponent::HandlerHttpRequest, this->mHttpComponent, this);
    }

    void HttpRemoteRequest::SetCode(HttpStatus code)
    {
        this->mHttpCode = code;
        NetWorkThread * httpThread = this->mHttpComponent->GetNetThread();
        httpThread->AddTask(&HttpRemoteSession::StartSendHttpMessage, this->mHttpSession);
    }

    bool HttpRemoteRequest::WriterToBuffer(std::ostream &os)
    {
        if(this->mWriteCount == 0)
        {
            os << this->mVersion << " " << (int) this->mHttpCode
               << " " << HttpStatusToString(this->mHttpCode) << "\r\n";
            if (this->mHttpContent != nullptr)
            {
                size_t size = this->mHttpContent->GetContentSize();
                os << "Content-Length:" << size << "\r\n";
                GKDebugWarning("content size = " << size);
            }
            auto iter = this->mHeardMap.begin();
            for (; iter != this->mHeardMap.end(); iter++)
            {
                const std::string &key = iter->first;
                const std::string &val = iter->second;
                os << key << ":" << val << "\r\n";
            }
            os << "\r\n";
        }
        this->mWriteCount++;
        if(this->mHttpContent == nullptr)
        {
            return true;
        }
        return this->mHttpContent->GetContent(os);
    }

    bool HttpRemoteRequest::SetHeard(const std::string &key, const std::string &val)
    {
        auto iter = this->mHeardMap.find(key);
        if(iter == this->mHeardMap.end())
        {
            this->mHeardMap.emplace(key, val);
            return true;
        }
        return false;
    }

    bool HttpRemoteRequest::SetContent(HttpContent *httpContent)
    {
        if (this->mHttpContent != nullptr || httpContent == nullptr)
        {
            return false;
        }
        this->mHttpContent = httpContent;
        return true;
    }
}