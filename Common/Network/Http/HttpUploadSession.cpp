//
// Created by zmhy0073 on 2021/10/26.
//

#include "HttpUploadSession.h"
#include <Core/App.h>
#include <Component/Scene/HttpComponent.h>
#include <Network/Http/Response/HttpGettHandler.h>
#include <Network/Http/Response/HttpPostHandler.h>
#include <Method/HttpServiceMethod.h>
namespace GameKeeper
{
    HttpUploadSession::HttpUploadSession(HttpUploadComponent *component)
    {
        this->mWriterCount = 0;
        this->mSocketProxy = nullptr;
        this->mUploadComponent = component;
        this->mFileContent = nullptr;
    }

    void HttpUploadSession::Clear()
    {
        HttpSessionBase::Clear();

        delete this->mSocketProxy;
        this->mWriterCount = 0;
        this->mSocketProxy = nullptr;
        this->mMethod.clear();
        this->mAddress.clear();
    }

    bool HttpUploadSession::WriterToBuffer(std::ostream & os)
    {
        HttpStatus code = HttpStatus::METHOD_NOT_ALLOWED;
        os << HttpVersion << (int) code << " " << HttpStatusToString(code) << "\r\n";
        os << "Server: " << "GameKeeper" << "\r\n";
        os << "Connection: " << "close" << "\r\n\r\n";
        return true;
    }

    void HttpUploadSession::Start(SocketProxy *socketProxy)
    {
        delete this->mSocketProxy;
        this->mSocketProxy = socketProxy;
        this->StartReceiveHead();
    }

    void HttpUploadSession::OnReceiveHeard(asio::streambuf & streamBuf)
    {
        std::string path;
        std::string version;
        std::istream is(&streamBuf);
        is >> this->mMethod >> path >> version;
    }

    void HttpUploadSession::SetCode(XCode code)
    {
        this->mCode = code;
        CoroutineComponent * corComponent = App::Get().GetCorComponent();
        MainTaskScheduler & taskScheduler = App::Get().GetTaskScheduler();

    }

    void HttpUploadSession::OnReceiveHeadAfter(XCode code)
    {
        if(code != XCode::Successful)
        {
            this->SetCode(code);
        }
        else
        {
            this->StartReceiveBody();
        }
    }

    void HttpUploadSession::StartReceiveBody()
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
                             std::bind(&HttpUploadSession::ReadBodyCallback, this, args1, args2));
        }
    }

    void HttpUploadSession::ReadBodyCallback(const asio::error_code &err, size_t size)
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
            AsioContext &context = this->mSocketProxy->GetContext();
            context.post(std::bind(&HttpUploadSession::StartReceiveBody, this));
        }
    }

    void HttpUploadSession::OnWriterAfter(XCode code)
    {

    }
}