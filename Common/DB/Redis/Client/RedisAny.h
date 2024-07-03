//
// Created by yjz on 2022/4/28.
//

#ifndef APP_REDISANY_H
#define APP_REDISANY_H
#include<string>
#include<iostream>
namespace redis
{
	class Any
	{
	 public:
        virtual ~Any() = default;
		virtual bool IsNumber() const = 0;
		virtual bool IsString() const = 0;
		virtual void Write(std::iostream & io) = 0;

        template<typename T>
        const T * Cast() const { return static_cast<const T *>(this);}
	};

	class Number : public Any
	{
	 public:
		Number(int value);
		Number(long long value);
	 public:
		void Write(std::iostream &io) final;
		bool IsNumber() const final { return true; }
		bool IsString() const final { return false;}
		long long GetValue() const { return this->mValue;}
	 private:
		long long mValue;
	};

	class String : public Any
	{
	 public:
		String();
		String(const size_t size);
		String(const std::string & value);
		String(const char * str, size_t size);
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

#endif //APP_REDISANY_H
