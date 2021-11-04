//
// Created by zmhy0073 on 2021/11/1.
//

#include "HttpRemoteRequestHandler.h"
#include <Core/App.h>
#include <Network/Http/HttpClientComponent.h>
#include <Network/Http/HttpRemoteSession.h>
namespace GameKeeper
{

    HttpRemoteRequestHandler::HttpRemoteRequestHandler(HttpClientComponent *component, HttpRemoteSession *session)
        : mHttpSession(session), mHttpComponent(component)
    {
        this->mWriteCount = 0;
        this->mHttpContent = nullptr;
#ifdef __DEBUG__
        this->mStartTime = TimeHelper::GetMilTimestamp();
#endif
    }

    HttpRemoteRequestHandler::~HttpRemoteRequestHandler() noexcept
    {
        delete this->mHttpSession;
        delete this->mHttpContent;
#ifdef __DEBUG__
        long long endTime = TimeHelper::GetMilTimestamp();
        GKDebugLog("http call " << this->mService << "." << this->mMethod
                                << " use time = " << ((endTime - this->mStartTime) / 1000.0f) << "s");
#endif
    }

    void HttpRemoteRequestHandler::OnSessionError(const asio::error_code &err)
    {
        XCode code = err == asio::error::eof
                ? XCode::Successful : XCode::HttpNetWorkError;
        this->SetCode(code);
    }

    void HttpRemoteRequestHandler::OnWriterAfter()
    {
        delete this;
    }

    void HttpRemoteRequestHandler::SetCode(XCode code)
    {
		this->mCode = code;
        MainTaskScheduler &taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.AddMainTask(&HttpClientComponent::HandlerHttpRequest, this->mHttpComponent, this);
    }

    void HttpRemoteRequestHandler::SetCode(HttpStatus code)
    {
        this->mHttpCode = code;
        NetWorkThread * httpThread = this->mHttpComponent->GetNetThread();
        httpThread->AddTask(&HttpRemoteSession::StartSendHttpMessage, this->mHttpSession);
    }

    bool HttpRemoteRequestHandler::WriterToBuffer(std::ostream &os)
    {
        if (this->mWriteCount == 0)
        {
            os << this->mVersion << " " << (int) this->mHttpCode
               << " " << HttpStatusToString(this->mHttpCode) << "\r\n";
            if (this->mHttpContent != nullptr)
            {
                size_t size = this->mHttpContent->GetContentSize();
                os << "Content-Length:" << size << "\r\n";
                GKDebugWarning("content size = " << size);
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

    bool HttpRemoteRequestHandler::SetHeard(const std::string &key, const std::string &val)
    {
        auto iter = this->mHeardMap.find(key);
        if(iter == this->mHeardMap.end())
        {
            this->mHeardMap.emplace(key, val);
            return true;
        }
        return false;
    }

    bool HttpRemoteRequestHandler::OnReceiveHeard(asio::streambuf &buf, size_t size)
    {
        std::string path;
        std::istream is(&buf);
        is >> path >> this->mVersion;
        this->ParseHeard(buf, size - (size - buf.size()));

        if (!this->ParseUrl(path))
        {
            this->SetCode(HttpStatus::NOT_FOUND);
            return false;
        }
        this->SetCode(XCode::Successful);
        return false;
    }

    bool HttpRemoteRequestHandler::SetContent(HttpWriteContent *httpContent)
    {
        if (this->mHttpContent != nullptr || httpContent == nullptr)
        {
            return false;
        }
        this->mHttpContent = httpContent;
        return true;
    }

    bool HttpRemoteRequestHandler::ParseUrl(const std::string & path)
    {
        const static std::string & app = "App/";
        size_t pos1 = path.find(app, 1);
        if (pos1 == std::string::npos)
        {
            return false;
        }
        size_t pos2 = path.find('/', pos1 + app.size());
        if (pos2 == std::string::npos)
        {
            return false;
        }
        size_t pos3 = path.find('/', pos2);
        size_t pos4 = path.find('/', pos3 + 1);
        if (pos3 == std::string::npos)
        {
            return false;
        }
        this->mService = path.substr(pos1 + app.size(), pos2 - app.size() - 1);
        this->mMethod = path.substr(pos3 + 1, pos4 - pos3 - 1);
        return true;
    }
}