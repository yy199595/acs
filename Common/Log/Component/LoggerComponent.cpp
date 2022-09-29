#include"LoggerComponent.h"
#include"App/App.h"
#include"spdlog/async.h"
#ifdef __DEBUG__
#include"Log/Debug.h"
#endif
#include"spdlog/sinks/rotating_file_sink.h"

namespace Sentry
{
	// 单线程  st  多线程  mt
	void LoggerComponent::Awake()
	{
		const ServerConfig& config = App::Get()->GetConfig();

		this->mLogSaveTime = 3;
		config.GetPath("log", this->mLogSavePath);
		config.GetMember("node_name", this->mServerName);
		this->CreateLogFile();
	}

	void LoggerComponent::OnZeroRefresh()
	{
		this->CreateLogFile();
	}

	void LoggerComponent::OnDestory()
	{
		this->SaveAllLog();
	}

	void LoggerComponent::SaveAllLog()
	{
#ifdef __DEBUG__
		this->mAllLog->flush();
#else
		this->mInfoLog->flush();
		this->mDebugLog->flush();
		this->mErrorLog->flush();
		this->mFatalLog->flush();
		this->mWarningLog->flush();
#endif
		spdlog::drop_all();
	}

	void LoggerComponent::AddLog(spdlog::level::level_enum type, const std::string& log)
    {
#ifdef __DEBUG__
        Debug::Console(type,
                       log);
#endif
        switch (type)
        {
            case spdlog::level::level_enum::info:
#ifdef __DEBUG__
                this->mAllLog->info(log);
#else
                this->mInfoLog->info(log);

#endif
                break;
            case spdlog::level::level_enum::warn:
#ifdef __DEBUG__
                this->mAllLog->warn(log);
#else
                this->mWarningLog->warn(log);

#endif
                break;
            case spdlog::level::level_enum::err:
#ifdef __DEBUG__
                this->mAllLog->error(log);
#else
                this->mErrorLog->error(log);

#endif
                break;
            case spdlog::level::level_enum::critical:
#ifdef __DEBUG__
                this->mAllLog->critical(log);
#else
                this->mFatalLog->critical(log);
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
}