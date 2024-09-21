//
// Created by zmhy0073 on 2021/12/2.
//

#include"RedisDefine.h"
#include"Async/Lua/LuaCoroutine.h"
#include"Yyjson/Document/Document.h"
namespace redis
{
	constexpr const char * CRLF = "\r\n";
    int Request::OnSendMessage(std::ostream& readStream)
    {
        if(this->mParameters.empty())
        {
            readStream << this->mCommand << CRLF;
            return 0;
        }
        readStream << "*" << this->mParameters.size() + 1 << CRLF;
        readStream << "$" << this->mCommand.size() << "\r\n" << this->mCommand << CRLF;
        for(const std::string & parameter : this->mParameters)
        {
			readStream << '$' << parameter.size() << CRLF << parameter << CRLF;
		}
		return 0;
    }

	void Request::Clear()
	{
		this->mCommand.clear();
		this->mParameters.clear();
	}

	std::string Request::ToString()
	{
		std::string json;
		json::w::Document document;
		std::unique_ptr<json::w::Value> list = document.AddArray(this->mCommand.c_str());
		{
			for(const std::string & str : this->mParameters)
			{
				list->Push(str.c_str(), str.size());
			}
		}
		document.Encode(&json);
		return json;
	}

	std::unique_ptr<Request> Request::MakeLua(const std::string& key, const std::string& func, const std::string & json)
	{
		std::unique_ptr<Request> request = std::make_unique<Request>();
		{
			request->SetCommand("EVALSHA");
			Request::InitParameter(request.get(), key, 1, func, json);
		}
		return request;
	}
}

namespace redis
{
    Response::Response()
        : mDataSize(0), mDataCount(0), mNumber(-1)
    {
		this->mIgnore = 0;
		this->mArray.clear();
        this->mType = Type::REDIS_NONE;
    }

	bool Response::HasError()
	{
		return this->mType == Type::REDIS_NONE
			   || this->mType == Type::REDIS_ERROR;
	}

	std::string Response::ToString()
	{
		switch(this->mType)
		{
			case Type::REDIS_NUMBER:
				return std::to_string(this->mNumber);
			case Type::REDIS_ERROR:
			case Type::REDIS_STRING:
			case Type::REDIS_BIN_STRING:
				if(this->mString.empty())
				{
					return "NULL";
				}
				return this->mString;
			case Type::REDIS_ARRAY:
			{
				json::w::Document json(true);
				for(const Any * any : this->mArray)
				{
					if(any->IsNumber())
					{
						json.Push(any->Cast<Number>()->GetValue());
					}
					else
					{
						json.Push(any->Cast<String>()->GetValue());
					}
				}
				std::string data;
				json.Encode(&data);
				return data;
			}
		}
		return "unknown data";
	}

    int Response::OnRecvLine(std::istream &message, size_t size)
    {
		while(this->mIgnore > 0)
		{
			if( message.ignore(this->mIgnore).gcount() == this->mIgnore)
			{
				this->mIgnore = 0;
				break;
			}
			return tcp::ReadOneLine;
		}
        switch (this->mType)
        {
            case Type::REDIS_NONE:
                return this->OnReceiveFirstLine(message, size);
            case Type::REDIS_ARRAY:
                return this->OnDecodeArray(message, size);
            default:
				return this->OnRecvMessage(message, size);
        }
    }

	void Response::SetError(const std::string& err)
	{
		this->mString.assign(err);
		this->mType = Type::REDIS_ERROR;
	}

	bool Response::GetArray(std::vector<const String*>& list)
	{
		if(this->mType != redis::Type::REDIS_ARRAY)
		{
			return false;
		}
		for(const Any * any : this->mArray)
		{
			if(any->IsString())
			{
				list.emplace_back(any->Cast<String>());
			}
		}
		return !list.empty();
	}

    int Response::OnRecvMessage(std::istream& os, size_t size)
	{
		size_t count = this->mDataSize;
		if (size < this->mDataSize)
		{
			count = size;
		}
		if (count > 0)
		{
			this->mDataSize -= count;
			std::unique_ptr<char[]> buffer(new char[count]);
			if (os.readsome(buffer.get(), count) != count)
			{
				this->SetError("read length");
				return tcp::ReadError;
			}
			switch (this->mType)
			{
				case Type::REDIS_STRING:
				case Type::REDIS_BIN_STRING:
				{
					this->mString.append(buffer.get(), count);
					break;
				}
				case Type::REDIS_ARRAY:
				{
					Any* redisAny = this->mArray.back();
					if (redisAny->IsString())
					{
						((String*)redisAny)->Append(buffer.get(), count);
					}
					break;
				}
				default:
					this->SetError("unknown type");
					return tcp::ReadError;
			}
		}
		if (this->mDataSize > 0)
		{
			return this->mDataSize;
		}
		this->mIgnore = 0;
		int len = os.ignore(2).gcount();
		if (len != 2)
		{
			this->mIgnore = 2 - len;
		}

		if (this->mArray.size() < this->mDataCount)
		{
			return tcp::ReadOneLine;
		}
		return tcp::ReadDone;
	}

    bool Response::IsOk()
    {
        return this->mString == "OK";
    }

    int Response::OnReceiveFirstLine(std::istream& os, size_t size)
	{
		this->mIgnore = 0;
		this->mDataSize = 0;
		char flag = (char)os.get();
		std::getline(os, this->mString);
		if(!this->mString.empty() && this->mString.back() == '\r')
		{
			this->mString.pop_back();
		}
		switch (flag)
		{
			case redis::Flag::String: //字符串类型
			{
				this->mType = Type::REDIS_STRING;
				return tcp::ReadDone;
			}
			case redis::Flag::Error: //错误
			{
				this->mType = Type::REDIS_ERROR;
				return tcp::ReadDone;
			}
			case redis::Flag::Number: //整型
			{
				this->mType = Type::REDIS_NUMBER;
				this->mNumber = std::stoll(this->mString);
				return tcp::ReadDone;
			}
			case redis::Flag::BinString: //二进制字符串
			{
				this->mType = Type::REDIS_BIN_STRING;
				this->mDataSize = std::stoi(this->mString);
				this->mString.clear();
				if (this->mDataSize == -1)
				{
					this->mString.clear();
					return tcp::ReadDone;
				}
				return this->mDataSize;
			}

			case redis::Flag::Array: //数组
				this->mType = Type::REDIS_ARRAY;
				this->mDataCount = std::stoi(this->mString);
				this->mString.clear();
				if (this->mDataCount == 0)
				{
					return tcp::ReadDone;
				}
				return tcp::ReadOneLine;
			default:
				this->mType = Type::REDIS_ERROR;
				char buff[512] = { 0 };
				size_t len = os.readsome(buff, sizeof(buff));
				CONSOLE_LOG_ERROR("unknown flag : {}:{}", (char)flag, std::string(buff, len));
				this->mString = fmt::format("unknown flag : {}:size", (char)flag, size);
				return tcp::ReadError;
		}
	}

    int Response::OnDecodeArray(std::istream & os, size_t)
    {
		this->mString.clear();
		char flag = (char)os.get();
		std::getline(os, this->mString);
		if(!this->mString.empty() && this->mString.back() == '\r')
		{
			this->mString.pop_back();
		}
		switch(flag)
		{
			case redis::Flag::String:
			{
				this->mArray.emplace_back(new String(this->mString));
				this->mString.clear();
				break;
			}
			case redis::Flag::BinString:
			{
				this->mDataSize = std::stoi(this->mString);
				if(this->mDataSize >= 0)
				{
					this->mArray.emplace_back(new String(this->mDataSize));
					return this->mDataSize + 2;
				}
				if(this->mDataSize == -1)
				{
					return tcp::ReadDone;
				}
				this->mArray.emplace_back(new String(this->mDataSize));
				return tcp::ReadOneLine;
			}
			case redis::Flag::Number:
			{
				long long number = std::stoll(this->mString);
				this->mArray.emplace_back(new Number(number));
				break;
			}
			case redis::Flag::Error:
				this->mType = Type::REDIS_ERROR;
				return tcp::ReadDone;
			case redis::Flag::Array:
				this->mType = Type::REDIS_ARRAY;
				this->mDataCount = std::stoi(this->mString);
				this->mString.clear();
				if (this->mDataCount == 0)
				{
					return tcp::ReadDone;
				}
				return tcp::ReadOneLine;
			default:
				this->mType = Type::REDIS_ERROR;
				this->mString = fmt::format("unknown type {}", flag);
				return tcp::ReadError;
		}
        return this->mArray.size() < this->mDataCount ? tcp::ReadOneLine : tcp::ReadDone;
    }

	void Response::Clear()
	{
		for(Any * redisAny : this->mArray)
		{
			delete redisAny;
		}
		this->mArray.clear();
		this->mType = Type::REDIS_NONE;
	}

	int Response::WriteToLua(lua_State* lua) const
	{
		switch (this->GetType())
		{
		case Type::REDIS_NUMBER:
			lua_pushinteger(lua, this->GetNumber());
			return 1;
		case Type::REDIS_ERROR:
		case Type::REDIS_STRING:
		case Type::REDIS_BIN_STRING:
		{
			const std::string& str = this->GetString();
			lua_pushlstring(lua, str.c_str(), str.size());
			return 1;
		}
		case Type::REDIS_ARRAY:
		{
			lua_createtable(lua, 0, (int)this->mArray.size());
			for (size_t index = 0; index < this->mArray.size(); index++)
			{
				const Any* redisAny = this->mArray[index];
				if (redisAny->IsString())
				{
					const std::string& str = ((const String*)redisAny)->GetValue();
					lua_pushlstring(lua, str.c_str(), str.size());
				}
				else if (redisAny->IsNumber())
				{
					long long num = ((const Number*)redisAny)->GetValue();
					lua_pushinteger(lua, num);
				}
				lua_seti(lua, -2, (int)index + 1);
			}
			return 1;
		}
		case Type::REDIS_NONE:
			lua_pushnil(lua);
			return 1;
		}
		return 0;
	}

	const Any* Response::Get(size_t index)
	{
		if(index < this->mArray.size())
		{
			return this->mArray[index];
		}
		return nullptr;
	}

	bool Response::GetValue(size_t index, std::string& value)
	{
		const Any * any = this->Get(index);
		if(any == nullptr || !any->IsString())
		{
			return false;
		}
		value = any->Cast<String>()->GetValue();
		return true;
	}
}

namespace acs
{
    RedisTask::RedisTask(int ms)
        : IRpcTask<redis::Response>(ms)
    {
		this->mMessage = nullptr;
    }

    LuaRedisTask::LuaRedisTask(lua_State *lua, int id)
        : IRpcTask<redis::Response>(id), mLua(lua)
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

    void LuaRedisTask::OnResponse(redis::Response * response)
	{
		int count = 0;
		lua_rawgeti(this->mLua, LUA_REGISTRYINDEX, this->mRef);
		lua_State* coroutine = lua_tothread(this->mLua, -1);
		if (response != nullptr)
		{
			count = response->WriteToLua(this->mLua);
		}
		else
		{
			count = 1;
			lua_pushnil(this->mLua);
		}
		Lua::Coroutine::Resume(coroutine, this->mLua, count);
	}
}
