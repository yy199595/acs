//
// Created by yjz on 2022/4/28.
//

#ifndef _REDISANY_H_
#define _REDISANY_H_
#include<string>
#include<iostream>
namespace Tendo
{
	class RedisAny
	{
	 public:
		virtual bool IsNumber() const = 0;
		virtual bool IsString() const = 0;
		virtual void Write(std::iostream & io) = 0;

        template<typename T>
        const T * Cast() const { return static_cast<const T *>(this);}
	};

	class RedisNumber : public RedisAny
	{
	 public:
		RedisNumber(int value);
		RedisNumber(long long value);
	 public:
		void Write(std::iostream &io) final;
		bool IsNumber() const final { return true; }
		bool IsString() const final { return false;}
		long long GetValue() const { return this->mValue;}
	 private:
		long long mValue;
	};

	class RedisString : public RedisAny
	{
	 public:
        RedisString();
        RedisString(const size_t size);
		RedisString(const std::string & value);
		RedisString(const char * str, size_t size);
	 public:
		void Write(std::iostream &io) final;
		void Append(const char * str, size_t size);
		bool IsNumber() const final { return false; }
		bool IsString() const final { return true;}
		const std::string & GetValue() const { return this->mValue;}
	 private:
		std::string mValue;
	};
}

#endif //_REDISANY_H_
