//
// Created by zmhy0073 on 2021/10/26.
//

#include"HttpDownloadRequestTask.h"
#include<Util/DirectoryHelper.h>
#include<Define/CommonDef.h>
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
        if(this->mFstream.is_open())
        {
            std::istream is(&streambuf);
            size_t size = is.readsome(this->mFileBuffer, 1024);
            while(size > 0)
            {
				this->mReadSize += size;
                this->mFstream.write(this->mFileBuffer, size);
                size = is.readsome(this->mFileBuffer, 1024);
				if (this->mFileSize > 0)
				{
					float process = (this->mReadSize / (float)this->mFileSize) * 100;
					SayNoDebugWarning("download " << this->mFileName << " from " << this->mUrl << " [" << process << "%]");
				}			
            }
        }
    }

	bool HttpDownloadRequestTask::OnReceiveHeard(const std::string & heard)
	{
		std::string path = this->mSavePath + this->mFileName;
		this->mFstream.open(path, std::ios::ate | std::ios::out);
		if (!this->mFstream.is_open())
		{
			return false;
		}

		std::string length;
		this->mFileSize = 0;
		this->mReadSize = 0;
		HttpRequestTask::OnReceiveHeard(heard);
		if (this->GetHeardData("Content-Length", length))
		{
			this->mFileSize = std::stoi(length);
		}
		return true;
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
        this->mSavePath = path;
        size_t pos = this->mUrl.find_last_of('/');
        this->mFileName = this->mUrl.substr(pos +1, this->mUrl.size() - pos);
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