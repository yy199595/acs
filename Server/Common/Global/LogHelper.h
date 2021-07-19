#pragma once
#include<spdlog/spdlog.h>
#include<Define/CommonTypeDef.h>
#include<spdlog/sinks/basic_file_sink.h>
#include<spdlog/sinks/stdout_color_sinks.h>
namespace Sentry
{
	using ELogType = spdlog::level::level_enum;

	class LogConsole
	{
	public:
		LogConsole() { }
		virtual ~LogConsole() { }
	public:
		virtual void ShowMessage(const std::string & log) = 0;
	};

	class LogInfoConsole : public LogConsole
	{
	public:
		void ShowMessage(const std::string & log) override;
	};

	class LogDebugConsole : public LogConsole
	{
	public:
		void ShowMessage(const std::string & log) override;
	};

	class LogErrorConsole : public LogConsole
	{
	public:
		void ShowMessage(const std::string & log) override;
	};

	class LogFatalConsole : public LogConsole
	{
	public:
		void ShowMessage(const std::string & log) override;
	};

	class LogWarningConsole : public LogConsole
	{
	public:	
		void ShowMessage(const std::string & log) override;
	};
}

namespace Sentry
{

	class LogHelper
	{
	public:
		LogHelper(const std::string path, const std::string name);
		~LogHelper() { }
	public:
		void DropLog() { spdlog::drop_all(); }
		void PushLog(ELogType type, const std::string & log);
		static void AddLog(ELogType type, const std::string & log);
	private:
		std::string mLogPath;
		std::shared_ptr<spdlog::logger> mLogger;
		std::unordered_map<ELogType, LogConsole *> mLoggerMap;
	private:
		static LogHelper * mLogHelper;
	};
}