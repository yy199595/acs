#pragma once

#include<Thread/TaskProxy.h>
#include<hiredis/hiredis.h>
#include<memory>
#include<list>
#include<string>
#include<vector>
#include"XCode/XCode.h"
#include<google/protobuf/message.h>

using namespace Sentry;
using namespace google::protobuf;

#define REDIS_SAVE_JSON

namespace Sentry
{
    class RedisComponent;

    typedef redisContext RedisSocket;
}// namespace Sentry

namespace Sentry
{
    enum class RedisRespType
    {
        REDIS_NONE,
        REDIS_STRING,
        REDIS_ERROR,
        REDIS_NUMBER,
        REDIS_ARRAY,
        REDIS_BIN_STRING,

    };
}

namespace Sentry
{
    class RedisAsioResp
    {
    public:
        RedisAsioResp();
        RedisRespType GetType() const { return this->mType;}

    public:
        int OnDecodeHead(std::iostream & readStream);
        void OnDecodeArray(std::iostream & readStream);
        void OnDecodeBinString(std::iostream & readStream);
        int OnReceiveFirstLine(char type, const std::string & lineData);
    private:
        int mDataCount;
        long long mNumber;
        RedisRespType mType;
        std::vector<std::string> mResponse;
    };
}

namespace Sentry
{
    class RedisCmdRequest
    {
    public:
        RedisCmdRequest(const std::string & cmd);

    public:
        void GetCommand(std::iostream & readStream) const;

        template<typename ... Args>
        void InitParameter(Args &&... args);
    public:
        void AddParameter(int value);
        void AddParameter(long long value);
        void AddParameter(const Message & message);
        void AddParameter(const std::string & value);
    private:
        void Encode() {}

        template<typename T, typename... Args>
        inline void Encode(const T &t, Args... args)
        {
            this->AddParameter(t);
            this->Encode(std::forward<Args>(args)...);
        }
    private:
        std::string mCommand;
        std::list<std::string> mParameters;
    };

    template<typename ... Args>
    void RedisCmdRequest::InitParameter(Args &&...args)
    {
        this->Encode(std::forward<Args>(args)...);
    }

    class RedisCmdResponse
    {
    public:
        bool IsOk();
        RedisCmdResponse();
        void AddValue(long long value);
        void AddValue(RedisRespType type);
        void AddValue(const std::string & data);
        void AddValue(const char * str, size_t size);
        const std::string & GetValue(size_t index = 0);

        bool HasError() { return this->mType == RedisRespType::REDIS_ERROR;}
    public:
        long long GetNumber() { return this->mNumber; }
        RedisRespType GetType() { return this->mType; }
        size_t GetArraySize() { return this->mDatas.size();}
    private:
        long long mNumber;
        RedisRespType mType;
        std::vector<std::string> mDatas;
    };
}

namespace Sentry
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
