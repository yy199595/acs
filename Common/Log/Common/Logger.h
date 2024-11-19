//
// Created by yy on 2023/8/12.
//

#ifndef APP_LOGGER_H
#define APP_LOGGER_H
#include"Level.h"
#include<Core/Map/HashMap.h>
#include<Core/Thread/AsioThread.h>
namespace custom
{
	struct FileConfig
	{
	public:
		int Save = 3; //自动保存时间 秒
		int MaxLine = 1024 * 10; // 多少行之后换文件
		int MaxSize = 1024 * 1024 * 20; //最大字节
		std::string Root; //根目录
		std::string Name;	//日志名字
	};

	struct LogConfig
	{
		int max_line = 0;
		int max_size = 0;
		bool open = true;
		bool console = false;
		std::string path;
		std::string name;
		std::string pem;
		std::string wx;
		std::string ding;
		std::string mongo;
		custom::LogLevel level;
	};

	class IOutput
	{
	public:
		IOutput() : mLevel(custom::LogLevel::None) { }
	public:
		void SetLevel(custom::LogLevel level) { this->mLevel = level; }
	public:
		virtual void Flush() { };
		virtual void Close() { };
		virtual void OnTick(int tick) { }
		virtual bool Start(Asio::Context & io) { return true; };
		virtual void Push(Asio::Context &io, const std::string & name, const LogInfo & logInfo) = 0;
	protected:
		custom::LogLevel mLevel;
	};

	class Logger
	{
	public:
		Logger(std::string name, Asio::Context & io);
	public:
		void Flush();
		bool Start();
		void Close();
		void SetLevel(custom::LogLevel level);
		void Push(std::unique_ptr<LogInfo> logInfo);
	public:

		template<typename T, typename ... Args>
		inline void AddOutput(Args&& ... args)
		{
			this->mOutputs.emplace_back(new T(std::forward<Args>(args)...));
		}
	private:
		void OnTimer(const Asio::Code & code);
	private:
		int mTick;
		int mSaveTime;
		std::string mName;
		Asio::Timer mTimer;
		Asio::Context & mContext;
		std::vector<IOutput *> mOutputs;
	};
}


#endif //APP_LOGGER_H
