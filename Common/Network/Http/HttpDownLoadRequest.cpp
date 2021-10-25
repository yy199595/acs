//
// Created by zmhy0073 on 2021/10/25.
//
#include <Core/App.h>

#include <Util/DirectoryHelper.h>
#include "HttpDownLoadRequest.h"
#include <Network/NetworkHelper.h>
namespace Sentry
{
    HttpDownLoadRequest::HttpDownLoadRequest(ISocketHandler * handler)
        : HttpRequest(handler, "HttpDownLoadRequest")
    {
        this->mCoreId = 0;
        this->mCurSize = 0;
        this->mMaxSize = 0;
        this->mFileStream = nullptr;
        this->mCorComponent = App::Get().GetCoroutineComponent();
    }

    HttpDownLoadRequest::~HttpDownLoadRequest() noexcept
    {
        if(this->mFileStream)
        {
            if(this->mFileStream->is_open())
            {
                this->mFileStream->close();
            }
            delete this->mFileStream;
        }
    }

    XCode HttpDownLoadRequest::DownLoad(const std::string &url, const std::string &path)
    {
        if(!NetworkHelper::ParseHttpUrl(url, this->mHost, this->mPort, this->mPath))
        {
            return XCode::HttpUrlParseError;
        }

        if(!DirectoryHelper::GetDirAndFileName(path, this->mSavePath, this->mFileName))
        {
            return XCode::Failure;
        }
        if(!DirectoryHelper::DirectorIsExist(this->mSavePath))
        {
            if(!DirectoryHelper::MakeDir(this->mSavePath))
            {
                return XCode::Failure;
            }
        }
        this->mFileStream = new std::fstream(path, std::ios::ate | std::ios::out);
        if(!this->mFileStream->is_open())
        {
            return XCode::Failure;
        }

        if(!this->ConnectRemote())
        {
            return XCode::HttpNetWorkError;
        }

        this->mCorComponent->YieldReturn(this->mCoreId);
        return XCode::Successful;
    }

    void HttpDownLoadRequest::OnReceiveDone(bool hasError)
    {
        this->mCode = hasError ? XCode::HttpNetWorkError : XCode::Successful;
        this->mMainTaskPool.AddMainTask(&CoroutineComponent::Resume, this->mCorComponent, this->mCoreId);
    }

    void HttpDownLoadRequest::OnReceiveBody(asio::streambuf &strem)
    {
        std::istream is(&strem);
        size_t size = is.readsome(this->buffer, 512);
        while(size > 0)
        {
            this->mCurSize += size;
            this->mFileStream->write(this->buffer, size);
            size = is.readsome(this->buffer, 1024);
            double process = this->mCurSize / (double )this->mMaxSize;
            SayNoDebugWarning("down load " << this->mFileName << "[" << process * 100 << "%]");
        }
    }

    void HttpDownLoadRequest::OnReceiveHeard(const std::string &heard)
    {
        const std::string str = "Content-Length:";
        size_t pos = heard.find(str);
        size_t pos1 = heard.find('\n', pos);
        this->mMaxSize = std::stoul(heard.substr(pos + str.size(), pos1 - pos));
    }

    void HttpDownLoadRequest::GetRquestParame(asio::streambuf &strem)
    {
        std::ostream os(&strem);
        os << "GET " << mPath << " HTTP/1.0\r\n";
        os << "Host: " << mHost << "\r\n";
        os << "Accept: */*\r\n";
        os << "Connection: close\r\n\r\n";
    }


}