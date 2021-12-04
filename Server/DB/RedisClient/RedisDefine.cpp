//
// Created by zmhy0073 on 2021/12/2.
//

#include"RedisDefine.h"
namespace GameKeeper
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
            if (reply->type != REDIS_REPLY_INTEGER)
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