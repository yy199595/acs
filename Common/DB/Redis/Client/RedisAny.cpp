//
// Created by yjz on 2022/4/28.
//

#include "RedisAny.h"
namespace redis
{
	Number::Number(int value)
		: mValue(value)
	{

	}

	Number::Number(long long value)
		: mValue(value)
	{

	}
	void Number::Write(std::iostream& io)
	{
		std::string str = std::to_string(this->mValue);
		io << '$' << str.size() << "\r\n" << str << "\r\n";
	}

    String::String()
    {

    }

	String::String(const size_t size)
	{
		this->mValue.reserve(size);
	}

	String::String(const std::string& value)
		: mValue(value)
	{

	}

	String::String(const char* str, size_t size)
		: mValue(str, size)
	{

	}

	void String::Write(std::iostream& io)
	{
		io << '$' << this->mValue.size() << "\r\n" << this->mValue << "\r\n";
	}
	void String::Append(const char* str, size_t size)
	{
		this->mValue.append(str, size);
	}
}
