//
// Created by yjz on 2022/4/28.
//

#include "RedisAny.h"
namespace Sentry
{
	RedisNumber::RedisNumber(int value)
		: mValue(value)
	{

	}

	RedisNumber::RedisNumber(long long value)
		: mValue(value)
	{

	}
	void RedisNumber::Write(std::iostream& io)
	{
		std::string str = std::to_string(this->mValue);
		io << '$' << str.size() << "\r\n" << str << "\r\n";
	}

	RedisString::RedisString(const size_t size)
	{
		this->mValue.reserve(size);
	}

	RedisString::RedisString(const std::string& value)
		: mValue(value)
	{

	}

	RedisString::RedisString(const char* str, size_t size)
		: mValue(str, size)
	{

	}

	void RedisString::Write(std::iostream& io)
	{
		io << '$' << this->mValue.size() << "\r\n" << this->mValue << "\r\n";
	}
	void RedisString::Append(const char* str, size_t size)
	{
		this->mValue.append(str, size);
	}
}
