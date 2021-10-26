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
        if (!NetworkHelper::ParseHttpUrl(url, this->mHost, this->mPort, this->mPath))
        {
            return XCode::HttpUrlParseError;
        }

        this->mSavePath = path;
        if (!DirectoryHelper::DirectorIsExist(path))
        {
            if (!DirectoryHelper::MakeDir(this->mSavePath))
            {
                return XCode::Failure;
            }
        }
        size_t pos = url.find_last_of('/');
        this->mFileName = path + url.substr(pos +1, url.size() - pos);
        if (!this->ConnectRemote())
        {
            return XCode::HttpNetWorkError;
        }

        this->mCorComponent->YieldReturn(this->mCoreId);
        return XCode::Successful;
    }

    void HttpDownLoadRequest::OnReceiveDone(bool hasError)
    {
        if(!hasError)
        {
            SayNoDebugInfo("down load file success");
        }
        this->mCode = hasError ? XCode::HttpNetWorkError : XCode::Successful;
        this->mMainTaskPool.AddMainTask(&CoroutineComponent::Resume, this->mCorComponent, this->mCoreId);
    }

    void HttpDownLoadRequest::OnReceiveBody(asio::streambuf &strem)
    {
        if(this->mFileStream == nullptr)
        {
            return;
        }
        std::istream is(&strem);
        size_t size = is.readsome(this->mDownLoadBuf, HTTP_DOWNLOAD_BUFFER);
        while(size > 0)
        {
            this->mCurSize += size;
            this->mFileStream->write(this->mDownLoadBuf, size);
            size = is.readsome(this->mDownLoadBuf, HTTP_DOWNLOAD_BUFFER);
            if(this->mMaxSize > 0)
            {
                double process = this->mCurSize / (double )this->mMaxSize;
                SayNoDebugWarning("down load " << this->mFileName << "[" << process * 100 << "%]");
            }
        }
    }

    void HttpDownLoadRequest::OnParseHeardDone()
    {
        if(this->GetHttpCode() != 200)
        {
            return;;
        }
        std::string type;
        std::string length;
        if (this->GetHeardData("Content-Length", length))
        {
            this->mMaxSize = std::stoul(length);
        }
        if(this->mFileName.find_last_of('.') == std::string::npos)
        {
            if (this->GetHeardData("Content-Type", type))
            {
                size_t pos1 = type.find('/');
                size_t pos2 = type.find(';');
                if (pos2 == std::string::npos)
                {
                    pos2 = type.size();
                }
                std::string t1 = type.substr(pos1 + 1, pos2 - pos1 - 1);
                this->mFileName += ("." + t1);
            }
        }
        this->mFileStream = new std::fstream(this->mFileName, std::ios::ate | std::ios::out);
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