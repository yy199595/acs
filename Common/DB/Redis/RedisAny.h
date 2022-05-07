//
// Created by yjz on 2022/4/28.
//

#ifndef _REDISANY_H_
#define _REDISANY_H_
#include<string>
#include<iostream>
namespace Sentry
{
	class RedisAny
	{
	 public:
		virtual bool IsLong() = 0;
		virtual bool IsString() = 0;
		virtual void Write(std::iostream & io) = 0;
	};

	class RedisLong : public RedisAny
	{
	 public:
		RedisLong(int value);
		RedisLong(long long value);
	 public:
		void Write(std::iostream &io) final;
		bool IsLong() final { return true; }
		bool IsString() final { return false;}
		long long GetValue() { return this->mValue;}
	 private:
		long long mValue;
	};

	class RedisString : public RedisAny
	{
	 public:
		RedisString(const std::string & value);
		RedisString(const char * str, size_t size);
	 public:
		void Write(std::iostream &io) final;
		bool IsLong() final { return false; }
		bool IsString() final { return true;}
		const std::string & GetValue() { return this->mValue;}
	 private:
		std::string mValue;
	};
}

#endif //_REDISANY_H_
