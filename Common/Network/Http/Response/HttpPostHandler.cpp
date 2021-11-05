//
// Created by zmhy0073 on 2021/11/4.
//

#include "HttpPostHandler.h"
#include <Core/App.h>
#include <Network/Http/HttpRemoteSession.h>
#include <Network/Http/HttpClientComponent.h>
#include <Scene/ProtocolComponent.h>
#include <Method/HttpServiceMethod.h>
namespace GameKeeper
{
    HttpPostHandler::HttpPostHandler(HttpClientComponent *component, HttpRemoteSession *session)
                                                               : HttpRequestHandler(component, session)
    {
        this->mContent = nullptr;
    }

    void HttpPostHandler::OnReceiveBodyAfter(XCode code)
    {
        this->mCode = code;
        if (this->mCode != XCode::Successful)
        {
            this->OnWriterAfter(code);
            return;
        }
        MainTaskScheduler &taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.AddMainTask(&HttpClientComponent::HandlerHttpRequest, this->mHttpComponent, this);
    }

    const std::string &HttpPostHandler::GetPath()
    {
        return this->mHttpSession->GetPath();
    }

    bool HttpPostHandler::OnReceiveHeard(asio::streambuf &buf, size_t size)
    {
        auto protocolComponent = App::Get().GetComponent<ProtocolComponent>();
        this->mHttpConfig = protocolComponent->GetHttpConfig(this->GetPath());
        if (this->mHttpConfig == nullptr)
        {
            this->SetCode(HttpStatus::NOT_FOUND);
            return false;
        }
        const std::string &method = this->mHttpConfig->Method;
        const std::string &service = this->mHttpConfig->Service;
        if(!this->mHttpComponent->GetHttpMethod(service, method))
        {
            this->SetCode(HttpStatus::NOT_FOUND);
            return false;
        }
        this->ParseHeard(buf, size);
        if (this->GetContentLength() == 0)
        {
            this->SetCode(HttpStatus::LENGTH_REQUIRED);
            return false;
        }
        std::string disposltion;
        if(this->GetHeardData("Content-Disposition", disposltion))
        {
            const std::string ss = "filename=";
            size_t pos = disposltion.find(ss);
            if (pos == std::string::npos)
            {
                this->SetCode(HttpStatus::PRECONDITION_FAILED);
                return false;
            }
            size_t offset = pos + ss.size();
            std::string file = disposltion.substr(offset, disposltion.size() - offset);
            const std::string path = App::Get().GetDownloadPath() + file;
            this->mContent = new HttpReadFileContent(path);
            return true;
        }
        this->mContent = new HttpReadStringContent();
        return true;
    }

    void HttpPostHandler::OnReceiveHeardAfter(XCode code)
    {
        this->mCode = code;
        if(this->mCode != XCode::Successful)
        {
            this->OnWriterAfter(code);
        }
    }


    void HttpPostHandler::OnReceiveBody(asio::streambuf &buf)
    {
        std::istream is(&buf);
        while(buf.size() > 0)
        {
            size_t size = is.readsome(this->mHandlerBuffer, 1024);
            this->mContent->OnReadContent(this->mHandlerBuffer, size);
        }
    }
}

