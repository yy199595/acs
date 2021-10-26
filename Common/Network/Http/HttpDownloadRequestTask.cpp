//
// Created by zmhy0073 on 2021/10/26.
//

#include "HttpDownloadRequestTask.h"
#include <Util/DirectoryHelper.h>
namespace Sentry
{
    void HttpDownloadRequestTask::GetSendData(asio::streambuf &streambuf)
    {
        std::ostream os(&streambuf);
        os << "GET " << mPath << " HTTP/1.0\r\n";
        os << "Host: " << mHost << "\r\n";
        os << "Accept: */*\r\n";
        os << "Connection: close\r\n\r\n";
    }

    void HttpDownloadRequestTask::OnReceiveBody(asio::streambuf &streambuf)
    {
        if(!this->mFstream.is_open())
        {
            this->mFstream.open(this->mFileName, std::ios::ate | std::ios::out);
        }
        if(this->mFstream.is_open())
        {
            char buffer[512];
            std::istream is(&streambuf);
            size_t size = is.readsome(buffer, 512);
            while(size > 0)
            {
                this->mFstream.write(buffer, size);
                size = is.readsome(buffer, 512);
            }
        }
    }

    XCode HttpDownloadRequestTask::Download(const std::string &path)
    {
        if (!DirectoryHelper::DirectorIsExist(path))
        {
            if (!DirectoryHelper::MakeDir(path))
            {
                return XCode::Failure;
            }
        }
        size_t pos = this->mUrl.find_last_of('/');
        this->mFileName = path + this->mUrl.substr(pos +1, this->mUrl.size() - pos);
        if(!this->AwaitInvoke())
        {
            return XCode::NoCoroutineContext;
        }

        if(this->mCode != XCode::Successful)
        {
            return this->mCode;
        }
        if(this->mHttpCode != 200)
        {
            return XCode::HttpResponseError;
        }
        return XCode::Successful;
    }
}