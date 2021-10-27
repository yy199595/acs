//
// Created by zmhy0073 on 2021/10/26.
//

#ifndef SENTRY_HTTPREQUESTTASK_H
#define SENTRY_HTTPREQUESTTASK_H
#include "Http.h"
#include <XCode/XCode.h>
#include <unordered_map>
#include <Thread/TaskProxy.h>
#include <Define/CommonTypeDef.h>
namespace Sentry
{
    class CoroutineComponent;

    class HttpRequestTask : public CoroutineAsyncTask
    {
    public:
        HttpRequestTask(const std::string &url);

        virtual ~HttpRequestTask()
        {}

    protected:
        virtual bool OnReceiveHeard(const std::string &heard);

        virtual void GetSendData(asio::streambuf &streambuf) = 0;

        virtual void OnReceiveBody(asio::streambuf &streambuf) = 0;

    public:
        bool Run() override;

        XCode GetCode() { return this->mCode; }
        const std::string & GetError() { return this->mError; }
        HttpStatus GetHttpCode() { return (HttpStatus) this->mHttpCode; }

        bool GetHeardData(const std::string &key, std::string &value);

    private:
        XCode Invoke();

    protected:
        XCode mCode;
        int mHttpCode;
        std::string mHost;
        std::string mPath;
        std::string mPort;
        const std::string mUrl;
    private:
        int mReadCount;
        bool mIsReadBody;
        std::string mError;
        std::string mVersion;
        std::unordered_map<std::string, std::string> mHeardMap;
    };
}
#endif //SENTRY_HTTPREQUESTTASK_H
