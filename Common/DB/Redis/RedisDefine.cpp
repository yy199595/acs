//
// Created by zmhy0073 on 2021/12/2.
//

#include"RedisDefine.h"
#include"Json/JsonWriter.h"
#include<google/protobuf/util/json_util.h>
namespace Sentry
{
    RedisRequest::RedisRequest(const std::string &cmd)
		: mCommand(cmd)
    {

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
		if(util::MessageToJsonString(message, &json).ok())
		{
			this->AddParameter(json);
		}
    }

    void RedisRequest::AddParameter(const std::string &value)
    {
		this->mParameters.emplace_back(value);
    }

    bool RedisRequest::Serailize(std::ostream& readStream)
    {
        if(this->mParameters.empty())
        {
            readStream << this->mCommand << "\r\n";
            return true;
        }
        readStream << "*" << this->mParameters.size() + 1 << "\r\n";
        readStream << "$" << this->mCommand.size() << "\r\n" << this->mCommand << "\r\n";
        for(const std::string & parameter : this->mParameters)
        {
			readStream << '$' << parameter.size() << "\r\n" << parameter << "\r\n";
		}
		return true;
    }

	const std::string RedisRequest::ToJson() const
	{
		Json::Writer jsonWriter;
		jsonWriter.StartArray(this->mCommand.c_str());
		for(const std::string & value : this->mParameters)
		{
			jsonWriter.AddMember(value);
		}
		jsonWriter.EndArray();
		return jsonWriter.ToJsonString();
	}

	std::shared_ptr<RedisRequest> RedisRequest::MakeLua(const std::string& key, const std::string& func, Json::Writer& json)
	{
		std::shared_ptr<RedisRequest> request = std::make_shared<RedisRequest>("EVALSHA");
		RedisRequest::InitParameter(request, key, 1, func, json.ToJsonString());
		return request;
	}
}

namespace Sentry
{
    RedisResponse::RedisResponse()
    {
        this->mType = RedisRespType::REDIS_NONE;
    }

	RedisResponse::~RedisResponse()
	{
		for(RedisAny * redisAny : this->mArray)
		{
			delete redisAny;
		}
		this->mArray.clear();
	}

    bool RedisResponse::IsOk()
    {
        if (this->HasError() || this->mArray.empty()
			|| !this->mArray.front()->IsString())
        {
            return false;
        }
		RedisString * redisString = (RedisString*)this->mArray.front();
		return redisString != nullptr && redisString->GetValue() == "OK";
    }

	long long RedisResponse::GetNumber(size_t index)
	{
		if(index < 0 || index >= this->mArray.size())
		{
			return -1;
		}
		if(!this->mArray[index]->IsLong())
		{
			return -1;
		}
		RedisLong * redisLong = (RedisLong*)this->mArray[index];
		return redisLong == nullptr ? -1 : redisLong->GetValue();
	}

    void RedisResponse::AddValue(long long value)
    {
		this->mArray.emplace_back(new RedisLong(value));
    }

    void RedisResponse::AddValue(RedisRespType type)
    {
        this->mType = type;
    }

    void RedisResponse::AddValue(const std::string &data)
    {
        this->mArray.emplace_back(new RedisString(data));
    }

    void RedisResponse::AddValue(const char *str, size_t size)
    {
        this->mArray.emplace_back(new RedisString(str, size));
    }

	bool RedisResponse::GetString(string& value, size_t index)
	{
		if(index < 0 || index >= this->mArray.size())
		{
			return false;
		}
		if(!this->mArray[index]->IsString())
		{
			return false;
		}
		value = ((RedisString*)this->mArray[index])->GetValue();
		return true;
	}

	void RedisResponse::Clear()
	{
		this->mArray.clear();
		this->mType = RedisRespType::REDIS_NONE;
	}
}
