//
// Created by zmhy0073 on 2021/12/16.
//

#ifndef GAMEKEEPER_HTTPRESPTASK_H
#define GAMEKEEPER_HTTPRESPTASK_H
#include<asio.hpp>
#include"Http/Http.h"
#include<unordered_map>
#include"Async/Task.h"
namespace GameKeeper
{
    class HttpRespTask : public Task<std::string>
    {
    public:
        HttpRespTask() = default;
        ~HttpRespTask() = default;
    public:
        void OnComplete(XCode code);
        void OnReceiveHead(asio::streambuf & buf);
        virtual bool OnReceiveBody(asio::streambuf & buf);
    public:
        HttpStatus AwaitGetCode();
        bool GetHead(const std::string & key, std::string & value) const;
    private:
        XCode mCode;
        std::string mError;
        HttpStatus mHttpCode;
        std::string mVersion;
        std::string mResponse;
        size_t mContentLength;
        std::unordered_map<std::string, std::string> mHeadMap;
    };
}


#endif //GAMEKEEPER_HTTPRESPTASK_H
