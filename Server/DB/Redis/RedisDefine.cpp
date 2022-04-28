//
// Created by zmhy0073 on 2021/12/2.
//

#include"RedisDefine.h"
#include"Pool/MessagePool.h"
#include"Json/JsonWriter.h"

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
        Helper::Proto::GetJson(message, json);
        this->AddParameter(json);
    }

    void RedisRequest::AddParameter(const std::string &value)
    {
		this->mParameters.emplace_back(value);
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
			readStream << '$' << parameter.size() << "\r\n" << parameter << "\r\n";
		}
    }

	const std::string RedisRequest::ToJson()
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

	std::shared_ptr<RedisRequest>
	RedisRequest::MakeLua(const std::string& key, const std::string& func,
		std::unordered_map<std::string, std::string>& parameters)
	{
		std::shared_ptr<RedisRequest> request
			= std::make_shared<RedisRequest>("EVALSHA");
		request->AddParameter(key);
		size_t size = parameters.size() + 1;
		request->AddParameter((int)size);
		request->AddParameter("func");
		for(auto iter = parameters.begin(); iter != parameters.end(); iter++)
		{
			request->AddParameter(iter->first);
		}
		request->AddParameter(func);

		for(auto iter = parameters.begin(); iter != parameters.end(); iter++)
		{
			request->AddParameter(iter->second);
		}
		return request;
	}
}

namespace Sentry
{
    RedisResponse::RedisResponse()
    {
        this->mType = RedisRespType::REDIS_NONE;
    }

    bool RedisResponse::IsOk()
    {
        if (this->HasError() && this->mArray.empty())
        {
            return false;
        }
		std::shared_ptr<RedisString> redisString = std::dynamic_pointer_cast<RedisString>(this->mArray.front());
		return redisString != nullptr && redisString->GetValue() == "OK";
    }

	long long RedisResponse::GetNumber(size_t index)
	{
		if(index < 0 || index >= this->mArray.size())
		{
			return false;
		}
		if(!this->mArray[index]->IsLong())
		{
			return false;
		}
		std::shared_ptr<RedisLong> redisLong = std::dynamic_pointer_cast<RedisLong>(this->mArray[index]);
		return redisLong == nullptr ? 0 : redisLong->GetValue();
	}

    void RedisResponse::AddValue(long long value)
    {
		this->mArray.emplace_back(std::make_shared<RedisLong>(value));
    }

    void RedisResponse::AddValue(RedisRespType type)
    {
        this->mType = type;
    }

    void RedisResponse::AddValue(const std::string &data)
    {
        this->mArray.emplace_back(std::make_shared<RedisString>(data));
    }

    void RedisResponse::AddValue(const char *str, size_t size)
    {
        this->mArray.emplace_back(std::make_shared<RedisString>(str, size));
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
		std::shared_ptr<RedisString> redisString = std::dynamic_pointer_cast<RedisString>(this->mArray[index]);
		value = redisString->GetValue();
		return true;
	}
}
