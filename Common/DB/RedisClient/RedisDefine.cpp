//
// Created by zmhy0073 on 2021/12/2.
//

#include"RedisDefine.h"
#include"Pool/MessagePool.h"
namespace Sentry
{
    RedisRequest::RedisRequest(const std::string &cmd)
    {
        this->mCommand = std::move(cmd);
    }

    void RedisRequest::AddParameter(int value)
    {
        this->mParameters.emplace_back(std::to_string(value));
    }

    void RedisRequest::AddParameter(long long value)
    {
        this->mParameters.emplace_back(std::to_string(value));
    }

    void RedisRequest::AddParameter(const Message &message)
    {
        std::string json;
        Helper::Proto::GetJson(message, json);
        this->AddParameter(json);
    }

    void RedisRequest::AddParameter(const std::string &value)
    {
        this->mParameters.emplace_back(std::move(value));
    }

    void RedisRequest::GetCommand(std::iostream &readStream) const
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
    RedisResponse::RedisResponse()
    {
        this->mNumber = 0;
        this->mType = RedisRespType::REDIS_NONE;
    }

    bool RedisResponse::IsOk()
    {
        if (this->HasError())
        {
            return false;
        }
        return !this->mArray.empty() &&
               this->mArray.front() == "OK";
    }

    void RedisResponse::AddValue(long long value)
    {
        this->mNumber = value;
        this->mType = RedisRespType::REDIS_NUMBER;
    }

    void RedisResponse::AddValue(RedisRespType type)
    {
        this->mType = type;
    }

    void RedisResponse::AddValue(const std::string &data)
    {
        this->mArray.emplace_back(std::move(data));
    }

    void RedisResponse::AddValue(const char *str, size_t size)
    {
        this->mArray.emplace_back(str, size);
    }

    const std::string &RedisResponse::GetValue(size_t index)
    {
        return this->mArray[index];
    }
}