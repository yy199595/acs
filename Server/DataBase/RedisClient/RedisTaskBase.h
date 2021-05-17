#pragma once
#include"RedisDefine.h"
#include<Thread/ThreadTaskAction.h>
#define RedisLuaArgvSize 10
namespace SoEasy
{
	class RedisManager;
	class QuertJsonWritre;
	class RedisTaskBase : public ThreadTaskAction 
	{
	public:
		RedisTaskBase(RedisManager * mgr, long long taskId, const std::string & cmd);
	public:
		void InvokeInThreadPool(long long threadId) final;	//在线程池执行的任务
	public:
		template<typename ...Args>
		void InitCommand(Args &&... args);
	private:
		inline void AddCommand(const char * value) { this->mCommand.push_back(value); }
		inline void AddCommand(const std::string & value) { this->mCommand.push_back(value); }
		inline void AddCommand(const long long value) { this->mCommand.push_back(std::to_string(value)); }
	private:
		void Encode() {}
		template<typename T, typename ... Args>
		inline void Encode(const T & t, Args ... args)
		{
			this->AddCommand(t);
			this->Encode(std::forward<Args>(args) ...);
		}
	public:
		void AddCommandArgv(const std::string & argv);
		void AddCommandArgv(const char * str, const size_t size);
	public:
		XCode GetErrorCode() { return this->mErrorCode; }
		const std::string & GetErrorStr() { return this->mErrorStr; }
	protected:
		virtual void OnQueryFinish(QuertJsonWritre & jsonWriter) = 0;
	private:
		std::vector<std::string> mCommand;
		const char * mArgvArray[RedisLuaArgvSize];
		size_t mArgvSizeArray[RedisLuaArgvSize];
	private:
		XCode mErrorCode;
		std::string mErrorStr;
	private:
		RedisManager * mRedisManager;
	};
	template<typename ...Args>
	inline void RedisTaskBase::InitCommand(Args && ...args)
	{
		this->Encode(std::forward<Args>(args)...);
	}
}