//
// Created by zmhy0073 on 2021/10/26.
//

#ifndef SENTRY_HTTPREQUESTTASK_H
#define SENTRY_HTTPREQUESTTASK_H
#include <XCode/XCode.h>
#include <asio.hpp>
#include <unordered_map>
#include <Thread/TaskProxy.h>
namespace Sentry
{
    class CoroutineComponent;
    class HttpRequestTask : public CoroutineAsyncTask
    {
    public:
        HttpRequestTask(const std::string & url);
        virtual ~HttpRequestTask() {}
    protected:
        virtual void GetSendData(asio::streambuf & streambuf) = 0;
        virtual void OnReceiveBody(asio::streambuf & streambuf) = 0;

        const std::string mUrl;
    public:
        bool Run() override;
        bool GetHeardData(const std::string & key, std::string & value);
    private:
        void ParseHeard(const std::string & heard);

    protected:
        XCode mCode;
        int mHttpCode;
        std::string mHost;
        std::string mPath;
    private:
        int mReadCount;
        bool mIsReadBody;
        std::string mError;
        std::string mVersion;
        std::unordered_map<std::string, std::string> mHeardMap;
    };
}
#endif //SENTRY_HTTPREQUESTTASK_H
