//
// Created by zmhy0073 on 2021/12/2.
//

#include"RedisDefine.h"
#include"Util/Json/JsonWriter.h"
#include"Async/Lua/LuaCoroutine.h"
#include"google/protobuf/util/json_util.h"
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

    int RedisRequest::Serialize(std::ostream& readStream)
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

    std::shared_ptr<RedisTask> RedisRequest::MakeTask(int id)
    {
        std::shared_ptr<RedisTask> redisTask = std::make_shared<RedisTask>(id);
        this->mTaskId = redisTask->GetRpcId();
        return redisTask;
    }

    std::shared_ptr<LuaRedisTask> RedisRequest::MakeLuaTask(lua_State *lua, int id)
    {
        std::shared_ptr<LuaRedisTask> redisTask(new LuaRedisTask(lua, id));
        this->mTaskId = redisTask->GetRpcId();
        return redisTask;
    }

	std::string RedisRequest::ToJson()
	{
		Json::Writer jsonWriter;
		jsonWriter.Add(this->mParameters);
		return jsonWriter.JsonString();
	}

	std::shared_ptr<RedisRequest> RedisRequest::MakeLua(const std::string& key, const std::string& func, const std::string & json)
	{
		std::shared_ptr<RedisRequest> request = std::make_shared<RedisRequest>("EVALSHA");
		RedisRequest::InitParameter(request, key, 1, func, json);
		return request;
	}
	void RedisRequest::AddString(const char* str, size_t size)
	{
		this->mParameters.emplace_back(str, size);
	}
}

namespace Sentry
{
    RedisResponse::RedisResponse()
        : mDataSize(0), mLineCount(0),
        mDataCount(0), mNumber(-1), mTaskId(0)
    {
        this->mType = RedisRespType::REDIS_NONE;
    }

	bool RedisResponse::HasError()
	{
		return this->mType == RedisRespType::REDIS_NONE
			   || this->mType == RedisRespType::REDIS_ERROR;
	}

	RedisResponse::~RedisResponse()
	{
		for(RedisAny * redisAny : this->mArray)
		{
			delete redisAny;
		}
		this->mArray.clear();
	}

    int RedisResponse::OnRecvLine(std::istream &message)
    {
        switch (this->mType)
        {
            case RedisRespType::REDIS_NONE:
                return this->OnReceiveFirstLine(message);
            case RedisRespType::REDIS_ARRAY:
                return this->OnDecodeArray(message);
            default:
                break;
        }
		return 0;
    }

    int RedisResponse::OnRecvMessage(std::istream& os)
	{
		this->mString.clear();
		std::unique_ptr<char[]> buffer(new char[this->mDataSize]);
		if (os.readsome(buffer.get(), this->mDataSize) == this->mDataSize)
        {
            switch (this->mType)
            {
                case RedisRespType::REDIS_STRING:
                case RedisRespType::REDIS_BIN_STRING:
                {
                    this->mString.append(buffer.get(),
                                         this->mDataSize);
                }
                    break;
                case RedisRespType::REDIS_ARRAY:
                {
                    RedisAny *redisAny = this->mArray[this->mLineCount++];
                    if (redisAny->IsString())
                    {
                        ((RedisString *) redisAny)->Append(buffer.get(),
                                                           this->mDataSize);
                    }
                    break;
                }
                default:
                    break;
            }
        }
		else
		{
			CONSOLE_LOG_FATAL("message lenght error");
			return 0;
		}
		os.ignore(2);
		return this->mLineCount < this->mDataCount ? -1 : 0; //再读一行
	}

    bool RedisResponse::IsOk()
    {
        return this->mString == "OK";
    }

    int RedisResponse::OnReceiveFirstLine(std::istream& os)
	{
		char cc = os.get();
		this->mDataSize = 0;
		std::getline(os, this->mString);
        if(this->mString.empty())
        {
            return 0;
        }
		this->mString.pop_back();
		switch (cc)
		{
		case '+': //字符串类型
			this->mType = RedisRespType::REDIS_STRING;
			break;
		case '-': //错误
			this->mType = RedisRespType::REDIS_ERROR;
			break;
		case ':': //整型
			this->mType = RedisRespType::REDIS_NUMBER;
			this->mNumber = std::stoll(this->mString);
			break;
		case '$': //二进制字符串
			this->mType = RedisRespType::REDIS_BIN_STRING;
			this->mDataSize = std::stoi(this->mString);
			if(this->mDataSize == -1)
			{
				this->mString.clear();
				return 0;
			}
			break;
		case '*': //数组
			this->mDataSize = -1;
			this->mType = RedisRespType::REDIS_ARRAY;
			this->mDataCount = std::stoi(this->mString);
            if(this->mDataCount == 0)
            {
                return 0;
            }
			break;
		}
		return this->mDataSize;
	}

    int RedisResponse::OnDecodeArray(std::istream & os)
    {
		char cc = os.get();
		this->mString.clear();
		if(cc == '$' && std::getline(os, this->mString))
		{
			this->mString.pop_back();
			this->mDataSize = std::stoi(this->mString);
            if(this->mDataSize >= 0)
            {
                this->mArray.emplace_back(new RedisString(this->mDataSize));
                return this->mDataSize + 2;
            }
            this->mArray.emplace_back(new RedisString());
            return -1;
		}
		else if(cc == ':' && std::getline(os, this->mString))
		{
			this->mLineCount++;
			this->mString.pop_back();
			long long number = std::stoll(this->mString);
			this->mArray.emplace_back(new RedisNumber(number));
		}
        return this->mLineCount < this->mDataCount ? -1 : 0;
    }

	void RedisResponse::Clear()
	{
		this->mArray.clear();
		this->mType = RedisRespType::REDIS_NONE;
	}

	const RedisAny* RedisResponse::Get(size_t index)
	{
		if(index < this->mArray.size())
		{
			return this->mArray[index];
		}
		return nullptr;
	}
}

namespace Sentry
{
    RedisTask::RedisTask(int ms)
        : IRpcTask<RedisResponse>(ms)
    {
    }

    void RedisTask::OnResponse(std::shared_ptr<RedisResponse> response)
    {
        this->mTask.SetResult(response);
    }

    LuaRedisTask::LuaRedisTask(lua_State *lua, int id)
        : IRpcTask<RedisResponse>(id), mLua(lua)
    {
        this->mRef = 0;
        if(lua_isthread(this->mLua, -1))
        {
            this->mRef = luaL_ref(lua, LUA_REGISTRYINDEX);
        }
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
		if (response != nullptr)
		{
			switch (response->GetType())
			{
				case RedisRespType::REDIS_NUMBER:
					lua_pushinteger(this->mLua, response->GetNumber());
					break;
				case RedisRespType::REDIS_ERROR:
				case RedisRespType::REDIS_STRING:
				case RedisRespType::REDIS_BIN_STRING:
				{
					const std::string& str = response->GetString();
					lua_pushlstring(this->mLua, str.c_str(), str.size());
				}
					break;
				case RedisRespType::REDIS_ARRAY:
				{
					lua_createtable(this->mLua, 0, response->GetArraySize());
					for (size_t index = 0; index < response->GetArraySize(); index++)
					{
						const RedisAny* redisAny = response->Get(index);
						if (redisAny->IsString())
						{
							const std::string& str = ((const RedisString*)redisAny)->GetValue();
							lua_pushlstring(this->mLua, str.c_str(), str.size());
						}
						else if (redisAny->IsNumber())
						{
							long long num = ((const RedisNumber*)redisAny)->GetValue();
							lua_pushinteger(this->mLua, num);
						}
						lua_seti(this->mLua, -2, index + 1);
					}
				}
					break;
				case RedisRespType::REDIS_NONE:
					lua_pushnil(this->mLua);
					break;
			}
		}
		else
		{
			lua_pushnil(this->mLua);
		}
		Lua::Coroutine::Resume(coroutine, this->mLua, 1);
	}
}
