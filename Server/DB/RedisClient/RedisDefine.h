#pragma once

#include<Thread/TaskProxy.h>
#include<hiredis/hiredis.h>
#include<memory>
#include<string>
#include<vector>
#include"XCode/XCode.h"
#include<google/protobuf/message.h>

using namespace GameKeeper;
using namespace google::protobuf;

#define REDIS_SAVE_JSON

namespace GameKeeper
{
    class RedisComponent;

    typedef redisContext RedisSocket;
}// namespace GameKeeper

namespace GameKeeper
{
    class RedisResponse
    {
    public:
         explicit RedisResponse(redisReply * reply);
         RedisResponse(XCode code, const std::string & err);
        ~RedisResponse();
    public:
        XCode GetCode() const { return this->mCode; }
        const std::string & GetError() const { return this->mError; }
        bool HasError() const { return this->mCode != XCode::Successful;}

    public:
        long long GetNumber();
        bool GetValue(Message & value);
        bool GetValue(long long & value);
        bool GetValue(std::string & value);
        bool GetValue(std::vector<long long> & value);
        bool GetValue(std::vector<std::string> & value);

    private:
        XCode mCode;
        std::string mError;
        redisReply * mResponse;
    };
}
