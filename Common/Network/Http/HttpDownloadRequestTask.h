//
// Created by zmhy0073 on 2021/10/26.
//

#ifndef SENTRY_HTTPDOWNLOADREQUESTTASK_H
#define SENTRY_HTTPDOWNLOADREQUESTTASK_H
#include <fstream>
#include "HttpRequestTask.h"
namespace Sentry
{
    class HttpDownloadRequestTask : public HttpRequestTask
    {
    public:
        using HttpRequestTask::HttpRequestTask;
        XCode Download(const std::string & path);
    protected:
        void GetSendData(asio::streambuf &streambuf) override;
        void OnReceiveBody(asio::streambuf &streambuf) override;
		bool OnReceiveHeard(const std::string & heard) override;
    private:
		size_t mReadSize;
		size_t mFileSize;
		std::string mSavePath;
        std::string mFileName;
        std::fstream mFstream;
		char mFileBuffer[1024];
    };
}

#endif //SENTRY_HTTPDOWNLOADREQUESTTASK_H
