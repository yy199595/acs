#include"LoggerComponent.h"
#include"App/App.h"
#include"spdlog/async.h"
#include"spdlog/sinks/stdout_color_sinks.h"
#include"spdlog/sinks/rotating_file_sink.h"
#include"Other/backward.hpp"
using namespace backward;
namespace Sentry
{
	// 单线程  st  多线程  mt
	void LoggerComponent::Awake()
	{
		const ServerConfig& config = App::Get()->GetConfig();
		LOG_CHECK_RET(config.GetMember("node_name", this->mServerName));
		LOG_CHECK_RET(config.GetMember("log", "path", this->mLogSavePath));
		LOG_CHECK_RET(config.GetMember("log", "save", this->mLogSaveTime));
		this->CreateLogFile();
	}

	void LoggerComponent::OnZeroRefresh()
	{
		this->CreateLogFile();
	}

	void LoggerComponent::OnDestory()
	{
		spdlog::drop_all();
	}

	void LoggerComponent::AddLog(spdlog::level::level_enum type, const std::string& log)
	{
		switch (type)
		{
		case spdlog::level::level_enum::info:
#ifdef __DEBUG__
			this->mAllLog->info(log);
			LoggerComponent::AddInfoLog(log);
#else

#endif
			break;
		case spdlog::level::level_enum::debug:
#ifdef __DEBUG__
			LoggerComponent::AddDebugLog(log);
			this->mAllLog->debug(log);
#else

#endif
			break;
		case spdlog::level::level_enum::warn:
#ifdef __DEBUG__
			this->mAllLog->warn(log);
			LoggerComponent::AddWarningLog(log);
#else

#endif
			break;
		case spdlog::level::level_enum::err:
#ifdef __DEBUG__
			this->mAllLog->error(log);
			LoggerComponent::AddErrorLog(log);
#else

#endif
			break;
		case spdlog::level::level_enum::critical:
#ifdef __DEBUG__
			this->mAllLog->critical(log);
			LoggerComponent::AddFatalLog(log);
#else

#endif
			break;
		}
	}

	void LoggerComponent::CreateLogFile()
	{
		spdlog::shutdown();
		spdlog::flush_every(std::chrono::seconds(this->mLogSaveTime));
		std::string logPath = fmt::format("{0}/{1}", this->mLogSavePath, Helper::Time::GetYearMonthDayString());
#ifndef ONLY_MAIN_THREAD
#ifdef __DEBUG__
		spdlog::set_level(spdlog::level::level_enum::debug);
		this->mAllLog = spdlog::rotating_logger_st<spdlog::async_factory>("All", logPath + "/all.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
#else
		this->mInfoLog = spdlog::rotating_logger_mt<spdlog::async_factory>("Info", logPath + "/info.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
		this->mDebugLog = spdlog::rotating_logger_mt<spdlog::async_factory>("Debug", logPath + "/debug.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
		this->mFatalLog = spdlog::rotating_logger_mt<spdlog::async_factory>("Fatal", logPath + "/fatal.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
		this->mErrorLog = spdlog::rotating_logger_mt<spdlog::async_factory>("Error", logPath + "/error.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
		this->mRecordLog = spdlog::rotating_logger_mt<spdlog::async_factory>("Record", logPath + "/record.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
		this->mWarningLog = spdlog::rotating_logger_mt<spdlog::async_factory>("Warning", logPath + "/warning.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
#endif
#else
#ifdef __DEBUG__
		spdlog::set_level(spdlog::level::level_enum::debug);
		this->mAllLog = spdlog::rotating_logger_st<spdlog::async_factory>("All",
			logPath + "/all.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
#else
		this->mInfoLog = spdlog::rotating_logger_st<spdlog::async_factory>("Info", logPath + "/info.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
		this->mDebugLog = spdlog::rotating_logger_st<spdlog::async_factory>("Debug", logPath + "/debug.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
		this->mFatalLog = spdlog::rotating_logger_st<spdlog::async_factory>("Fatal", logPath + "/fatal.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
		this->mErrorLog = spdlog::rotating_logger_st<spdlog::async_factory>("Error", logPath + "/error.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
		this->mRecordLog = spdlog::rotating_logger_st<spdlog::async_factory>("Record", logPath + "/record.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
		this->mWarningLog = spdlog::rotating_logger_st<spdlog::async_factory>("Warning", logPath + "/warning.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
#endif

#endif
	}

	void LoggerComponent::AddInfoLog(const std::string& log)
	{
#ifdef _WIN32
		std::string time = Helper::Time::GetDateString();
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
			FOREGROUND_BLUE | FOREGROUND_GREEN |
			FOREGROUND_INTENSITY);
		printf("%s [Info   ] %s\n", time.c_str(), log.c_str());
#else
		std::string time = Helper::Time::GetDateString();
		printf("%s%s [Info   ] %s\e[34m\n", "\e[1m", time.c_str(), log.c_str());
#endif
	}

	void LoggerComponent::AddErrorLog(const std::string& log)
	{

#ifdef _WIN32
		std::string time = Helper::Time::GetDateString();
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
			FOREGROUND_INTENSITY | 4);
		printf("%s [Error  ] %s\n", time.c_str(), log.c_str());
#else
		std::string time = Helper::Time::GetDateString();
		printf("%s%s [Error  ] %s\e[0m\n", "\e[31m", time.c_str(), log.c_str());
#endif
	}

	void LoggerComponent::AddDebugLog(const std::string& log)
	{

#ifdef _WIN32
		std::string time = Helper::Time::GetDateString();
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
			FOREGROUND_INTENSITY | 2);
		printf("%s [Debug  ] %s\n", time.c_str(), log.c_str());
#else
		std::string time = Helper::Time::GetDateString();
		printf("%s%s [Debug  ] %s\e[0m\n", "\e[32m", time.c_str(), log.c_str());
#endif
	}

	void LoggerComponent::AddFatalLog(const std::string& log)
	{
#ifdef _WIN32
		std::string time = Helper::Time::GetDateString();
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
			FOREGROUND_INTENSITY | FOREGROUND_BLUE |
			FOREGROUND_RED);
		printf("%s [Fatal  ] %s\n", time.c_str(), log.c_str());
#else
		std::string time = Helper::Time::GetDateString();
		printf("%s%s [Fatal  ] %s\e[0m\n", "\e[35m", time.c_str(), log.c_str());
#endif
		StackTrace st;
		st.load_here(64);
		Printer p;
		p.color_mode = ColorMode::always;
		p.print(st, stderr);
	}

	void LoggerComponent::AddWarningLog(const std::string& log)
	{

#ifdef _WIN32
		std::string time = Helper::Time::GetDateString();
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
			FOREGROUND_INTENSITY | 6);
		printf("%s [Warning] %s\n", time.c_str(), log.c_str());
#else
		std::string time = Helper::Time::GetDateString();
		printf("%s%s [Warning] %s\e[0m\n", "\e[33m", time.c_str(), log.c_str());
#endif
	}
}