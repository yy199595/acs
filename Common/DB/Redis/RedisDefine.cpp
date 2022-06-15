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
        this->mTaskId = 0;
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

    int RedisRequest::Serailize(std::ostream& readStream)
    {
        if(this->mParameters.empty())
        {
            readStream << this->mCommand << "\r\n";
            return 0;
        }
        readStream << "*" << this->mParameters.size() + 1 << "\r\n";
        readStream << "$" << this->mCommand.size() << "\r\n" << this->mCommand << "\r\n";
        for(const std::string & parameter : this->mParameters)
        {
			readStream << '$' << parameter.size() << "\r\n" << parameter << "\r\n";
		}
		return 0;
    }

    std::shared_ptr<RedisTask> RedisRequest::MakeTask()
    {
        std::shared_ptr<RedisTask> redisTask(new RedisTask());
        this->mTaskId = redisTask->GetRpcId();
        return redisTask;
    }

    std::shared_ptr<LuaRedisTask> RedisRequest::MakeLuaTask(lua_State *lua)
    {
        std::shared_ptr<LuaRedisTask> redisTask(new LuaRedisTask(lua));
        this->mTaskId = redisTask->GetRpcId();
        return redisTask;
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

	std::shared_ptr<RedisRequest> RedisRequest::MakeLua(const std::string& key, const std::string& func, const std::string & json)
	{
		std::shared_ptr<RedisRequest> request = std::make_shared<RedisRequest>("EVALSHA");
		RedisRequest::InitParameter(request, key, 1, func, json);
		return request;
	}
}

namespace Sentry
{
    RedisResponse::RedisResponse(long long taskId)
        : mTaskId(taskId), mLineCount(0), mDataSize(0),
        mNumber(-1), mDataCount(0)
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

    bool RedisResponse::OnReceive(const asio::error_code &code, asio::streambuf &stream)
    {
        if (code || stream.size() == 0)
        {
            return true;
        }
        std::iostream readStream(&stream);
        switch (this->mType)
        {
            case RedisRespType::REDIS_NONE:
                this->OnDecodeHead(readStream);
                break;
        }
        this->OnDecodeMessage(readStream, stream.size());
        return this->mLineCount >= this->mDataCount;
    }

    void RedisResponse::OnDecodeMessage(std::iostream &readStream, size_t size)
    {
        switch (this->mType)
        {
            case RedisRespType::REDIS_BIN_STRING:
                this->OnDecodeBinString(readStream, size);
                break;
            case RedisRespType::REDIS_ARRAY:
                this->OnDecodeArray(readStream, size);
                break;
        }
    }

    bool RedisResponse::IsOk()
    {
        return this->mString == "OK";
    }

    void RedisResponse::OnDecodeHead(std::iostream &readStream)
    {
        std::string lineData;
        char type = readStream.get();
        if (std::getline(readStream, lineData))
        {
            lineData.pop_back(); //拿掉\r
            this->mDataCount = this->OnReceiveFirstLine(type, lineData);
        }
    }

    int RedisResponse::OnReceiveFirstLine(char type, const std::string &lineData)
    {
        switch (type)
        {
            case '+': //字符串类型
                //STD_ERROR_LOG("str = " << lineData.data());
                this->mType = RedisRespType::REDIS_STRING;
                this->mString = std::move(lineData);
                break;
            case '-': //错误
                this->mType = RedisRespType::REDIS_ERROR;
                this->mString = std::move(lineData);
                break;
            case ':': //整型
                this->mType = RedisRespType::REDIS_NUMBER;
                this->mNumber = std::stoll(lineData);
                break;
            case '$': //二进制字符串
                this->mDataSize = std::stoi(lineData);
                this->mType = RedisRespType::REDIS_BIN_STRING;
                return this->mDataSize <= 0 ? 0 : 1;
            case '*': //数组
                this->mType = RedisRespType::REDIS_ARRAY;
                return std::stoi(lineData);
        }
        return 0;
    }

    void RedisResponse::OnDecodeBinString(std::iostream &readStream, size_t size)
    {
        if (size >= this->mDataSize)
        {
            std::unique_ptr<char[]> buffer(new char[this->mDataSize]);
            if (readStream.readsome(buffer.get(), this->mDataSize) > 0)
            {
                switch (this->mType)
                {
                    case RedisRespType::REDIS_ARRAY:
                        this->mArray.emplace_back(new RedisString(buffer.get(), this->mDataSize));
                        break;
                    case RedisRespType::REDIS_BIN_STRING:
                        this->mString.append(buffer.get(), this->mDataSize);
                        break;
                }
            }
            this->mLineCount++;
            readStream.ignore(2);
        }
    }

    void RedisResponse::OnDecodeArray(std::iostream &readStream, size_t size)
    {
        std::string lineData;
        char type = readStream.get();
        if (type == '$' && std::getline(readStream, lineData))
        {
            lineData.pop_back();
            this->mDataSize = std::stoi(lineData);
            this->OnDecodeBinString(readStream, size);
        }
        else if (type == ':' && std::getline(readStream, lineData))
        {
            lineData.pop_back();
            this->mLineCount++;
            long long number = std::stoll(lineData);
            this->mArray.emplace_back(new RedisNumber(number));
        }
        if(this->mLineCount < this->mDataCount)
        {
            this->OnDecodeMessage(readStream, size);
        }
    }

	void RedisResponse::Clear()
	{
		this->mArray.clear();
		this->mType = RedisRespType::REDIS_NONE;
	}

	const RedisAny* RedisResponse::Get(size_t index)
	{
		if(index < this->mArray.size() || index >= 0)
		{
			return this->mArray[index];
		}
		return nullptr;
	}
}

namespace Sentry
{
    RedisTask::RedisTask()
    {
        this->mTaskId = Helper::Guid::Create();
    }

    void RedisTask::OnResponse(std::shared_ptr<RedisResponse> response)
    {
        this->mTask.SetResult(response);
    }

    LuaRedisTask::LuaRedisTask(lua_State *lua)
        : mLua(lua)
    {
        this->mRef = 0;
        if(lua_isthread(this->mLua, -1))
        {
            this->mRef = luaL_ref(lua, LUA_REGISTRYINDEX);
        }
        this->mTaskId = Helper::Guid::Create();
    }
    LuaRedisTask::~LuaRedisTask()
    {
        if(this->mRef != 0)
        {
            luaL_unref(this->mLua, LUA_REGISTRYINDEX, this->mRef);
        }
    }

    int LuaRedisTask::Await()
    {
        if(this->mRef == 0)
        {
            luaL_error(this->mLua, "not lua coroutine context yield failure");
            return 0;
        }
        return lua_yield(this->mLua, 0);
    }

    void LuaRedisTask::OnResponse(std::shared_ptr<RedisResponse> response)
    {
        lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
        lua_State* coroutine = lua_tothread(this->mLua, -1);

        switch(response->GetType())
        {
            case RedisRespType::REDIS_NUMBER:
                lua_pushinteger(this->mLua, response->GetNumber());
                break;
            case RedisRespType::REDIS_ERROR:
            case RedisRespType::REDIS_STRING:
            case RedisRespType::REDIS_BIN_STRING:
            {
                const std::string &str = response->GetString();
                lua_pushlstring(this->mLua, str.c_str(), str.size());
            }
                break;
            case RedisRespType::REDIS_ARRAY:
            {
                lua_createtable(this->mLua, 0, response->GetArraySize());
                for(size_t index = 0; index < response->GetArraySize(); index++)
                {
                    const RedisAny * redisAny = response->Get(index);
                    if(redisAny->IsString())
                    {
                        const std::string & str = ((const RedisString*)redisAny)->GetValue();
                        lua_pushlstring(this->mLua, str.c_str(), str.size());
                    }
                    else if(redisAny->IsNumber())
                    {
                        long long num = ((const RedisNumber*)redisAny)->GetValue();
                        lua_pushinteger(this->mLua, num);
                    }
                    lua_seti(this->mLua, -2, index + 1);
                }
            }
                break;
        }
        lua_presume(coroutine, this->mLua, 1);
    }
}
