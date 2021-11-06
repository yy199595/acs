//
// Created by zmhy0073 on 2021/11/1.
//

#include "HttpRequestHandler.h"
#include <Core/App.h>
#include <Other/ProtocolConfig.h>
#include <Network/Http/HttpRemoteSession.h>
#include <Network/Http/HttpClientComponent.h>
#include <Network/Http/Content/HttpReadContent.h>
#include <Network/Http/Content/HttpWriteContent.h>
namespace GameKeeper
{

    HttpRequestHandler::HttpRequestHandler(HttpClientComponent *component, HttpRemoteSession *session)
        : mHttpSession(session), mHttpComponent(component)
    {
        this->mWriteCount = 0;
        this->mHttpConfig = nullptr;
        this->mHttpContent = nullptr;
#ifdef __DEBUG__
        this->mStartTime = TimeHelper::GetMilTimestamp();
#endif
    }

    HttpRequestHandler::~HttpRequestHandler() noexcept
    {
        delete this->mHttpSession;
        delete this->mHttpContent;
#ifdef __DEBUG__
        if(this->mHttpConfig != nullptr)
        {
            long long endTime = TimeHelper::GetMilTimestamp();
            GKDebugLog("http call " << this->mHttpConfig->Service << "." << this->mHttpConfig->Method
                                    << " use time = " << ((endTime - this->mStartTime) / 1000.0f) << "s");
        }
#endif
    }

    void HttpRequestHandler::OnWriterAfter(XCode code)
    {
        delete this;
    }

    void HttpRequestHandler::SetCode(HttpStatus code)
    {
        this->mHttpCode = code;
		NetWorkThread & httpThread = this->GetSession()->GetThread();
        if(httpThread.IsCurrentThread())
        {
            this->mHttpSession->StartSendHttpMessage();
            return;
        }
        httpThread.AddTask(&HttpRemoteSession::StartSendHttpMessage, this->mHttpSession);
    }

    bool HttpRequestHandler::WriterToBuffer(std::ostream &os)
    {
        if (this->mWriteCount == 0)
        {
            os << this->mVersion << " " << (int) this->mHttpCode
               << " " << HttpStatusToString(this->mHttpCode) << "\r\n";
            if (this->mHttpContent != nullptr)
            {
                this->mHttpContent->GetContentType(os);
                os << "Content-Length:" << this->mHttpContent->GetContentSize() << "\r\n";
            }
            os << "Server:" << "GameKeeper" << "\r\n";
            os << "Connection:close" << "\r\n";
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
        if (this->mHttpContent == nullptr)
        {
            return true;
        }
        return this->mHttpContent->GetContent(os);
    }

    bool HttpRequestHandler::SetHeard(const std::string &key, const std::string &val)
    {
        auto iter = this->mHeardMap.find(key);
        if(iter == this->mHeardMap.end())
        {
            this->mHeardMap.emplace(key, val);
            return true;
        }
        return false;
    }


    bool HttpRequestHandler::SetContent(HttpWriteContent *httpContent)
    {
        if (this->mHttpContent != nullptr || httpContent == nullptr)
        {
            return false;
        }
        this->mHttpContent = httpContent;
        return true;
    }
}