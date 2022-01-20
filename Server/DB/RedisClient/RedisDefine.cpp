//
// Created by zmhy0073 on 2021/12/2.
//

#include"RedisDefine.h"
#include"Pool/MessagePool.h"
namespace Sentry
{
    RedisCmdRequest::RedisCmdRequest(const std::string &cmd)
    {
        this->mCommand = std::move(cmd);
    }

    void RedisCmdRequest::AddParameter(int value)
    {
        this->mParameters.emplace_back(std::to_string(value));
    }

    void RedisCmdRequest::AddParameter(long long value)
    {
        this->mParameters.emplace_back(std::to_string(value));
    }

    void RedisCmdRequest::AddParameter(const Message &message)
    {
        std::string json;
        Helper::Proto::GetJson(message, json);
        this->AddParameter(json);
    }

    void RedisCmdRequest::AddParameter(const std::string &value)
    {
        this->mParameters.emplace_back(std::move(value));
    }

    void RedisCmdRequest::GetCommand(std::iostream &readStream) const
    {
        if(this->mParameters.empty())
        {
            readStream << this->mCommand << "\r\n";
            return;
        }
        readStream << "*" << this->mParameters.size() + 1 << "\r\n";
        readStream << "$" << this->mCommand.size() << "\r\n" << this->mCommand << "\r\n";
        for(const std::string & parameter : this->mParameters)
        {
            readStream << "$" << parameter.size() << "\r\n" << parameter << "\r\n";
        }
    }
}

namespace Sentry
{
    RedisCmdResponse::RedisCmdResponse()
    {
        this->mNumber = 0;
        this->mType = RedisRespType::REDIS_NONE;
    }

    bool RedisCmdResponse::IsOk()
    {
        if (this->HasError())
        {
            return false;
        }
        return !this->mDatas.empty() &&
               this->mDatas.front() == "OK";
    }

    void RedisCmdResponse::AddValue(long long value)
    {
        this->mNumber = value;
        this->mType = RedisRespType::REDIS_NUMBER;
    }

    void RedisCmdResponse::AddValue(RedisRespType type)
    {
        this->mType = type;
    }

    void RedisCmdResponse::AddValue(const std::string &data)
    {
        this->mDatas.emplace_back(std::move(data));
    }

    void RedisCmdResponse::AddValue(const char *str, size_t size)
    {
        this->mDatas.emplace_back(str, size);
    }

    const std::string &RedisCmdResponse::GetValue(size_t index)
    {
        return this->mDatas[index];
    }
}

namespace Sentry
{
    RedisResponse::RedisResponse(redisReply *reply)
    {
        this->mResponse = reply;
        this->mCode = XCode::Successful;
    }

    RedisResponse::RedisResponse(XCode code, const std::string & err)
    {
        this->mCode = code;
        this->mError = err;
        this->mResponse = nullptr;
    }

    RedisResponse::~RedisResponse()
    {
        if(this->mResponse)
        {
            freeReplyObject(this->mResponse);
        }
    }

    long long RedisResponse::GetNumber()
    {
        if (this->mResponse == nullptr || this->mCode != XCode::Successful
            || this->mResponse->type != REDIS_REPLY_INTEGER)
        {
            return 0;
        }
        return this->mResponse->integer;
    }

    bool RedisResponse::GetValue(long long int &value)
    {
        if (this->mResponse == nullptr || this->mCode != XCode::Successful
            || this->mResponse->type != REDIS_REPLY_INTEGER)
        {
            return false;
        }
        value = this->mResponse->integer;
        return true;
    }

    bool RedisResponse::GetValue(std::string &value)
    {
        if (this->mResponse == nullptr || this->mCode != XCode::Successful
            || this->mResponse->type != REDIS_REPLY_STRING)
        {
            return false;
        }

        value.append(this->mResponse->str, this->mResponse->len);
        return true;
    }

    bool RedisResponse::GetValue(std::vector<long long int> &value)
    {
        if (this->mResponse == nullptr || this->mCode != XCode::Successful
            || this->mResponse->type != REDIS_REPLY_ARRAY)
        {
            return false;
        }
        for (int index = 0; index < this->mResponse->elements; index++)
        {
            redisReply *reply = this->mResponse->element[index];
            if (reply->type != REDIS_REPLY_INTEGER)
            {
                return false;
            }
            value.emplace_back(reply->integer);
        }
        return true;
    }

    bool RedisResponse::GetValue(std::vector<std::string> &value)
    {
        if (this->mResponse == nullptr || this->mCode != XCode::Successful
            || this->mResponse->type != REDIS_REPLY_ARRAY)
        {
            return false;
        }
        for (int index = 0; index < this->mResponse->elements; index++)
        {
            redisReply *reply = this->mResponse->element[index];
            if(reply->type != REDIS_REPLY_STRING)
            {
                return false;
            }
            value.emplace_back(reply->str, reply->len);
        }
        return true;
    }

    bool RedisResponse::GetValue(Message &value)
    {
        if (this->mResponse == nullptr || this->mCode != XCode::Successful
            || this->mResponse->type != REDIS_REPLY_STRING)
        {
            return false;
        }
        return value.ParseFromArray(this->mResponse->str, this->mResponse->len);
    }
}

namespace Sentry
{
    RedisAsioResp::RedisAsioResp()
    {
        this->mDataCount = 0;
        this->mType = RedisRespType::REDIS_NONE;
    }


    void RedisAsioResp::OnDecodeArray(std::iostream &readStream)
    {

    }

    void RedisAsioResp::OnDecodeBinString(std::iostream &readStream)
    {

    }

    int RedisAsioResp::OnReceiveFirstLine(char type, const std::string &lineData)
    {

    }

    int RedisAsioResp::OnDecodeHead(std::iostream &readStream)
    {
        std::string lineData;
        char type = readStream.get();
        if (std::getline(readStream, lineData))
        {
            lineData.pop_back(); //拿掉\r
            switch(type)
            {
                case '+': //字符串类型
                    this->mType = RedisRespType::REDIS_STRING;
                    this->mResponse.emplace_back(std::move(lineData));
                    return 0;
                case '-': //错误
                    this->mType = RedisRespType::REDIS_ERROR;
                    this->mResponse.emplace_back(std::move(lineData));
                    return 0;
                case ':': //整型
                    this->mNumber = std::stoll(lineData);
                    this->mType = RedisRespType::REDIS_NUMBER;
                    return 0;
                case '$': //二进制字符串
                    this->mType = RedisRespType::REDIS_BIN_STRING;
                    return std::stoi(lineData);
                    return 1;
                case '*': //数组
                    this->mType = RedisRespType::REDIS_ARRAY;
                    return std::stoi(lineData);
            }
            return 0;
        }
    }
}