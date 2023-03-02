#include"LoggerComponent.h"
#include"App/App.h"
#include"spdlog/async.h"
#ifdef __DEBUG__
#include"Log/Debug.h"
#endif
#include"System/System.h"
#include"spdlog/sinks/rotating_file_sink.h"

namespace Sentry
{
	// 单线程  st  多线程  mt
	bool LoggerComponent::Awake()
	{
		this->mLogSaveTime = 3;
        this->mServerName = ServerConfig::Inst()->Name();
        this->mLogSavePath = fmt::format("{0}/log", System::WorkPath());
		this->CreateLogFile();
        return true;
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
		this->mAllLog->flush();
		spdlog::drop_all();
	}

	void LoggerComponent::AddLog(spdlog::level::level_enum type, const std::string& log)
    {
        this->mAllLog->log(type, log);
        switch (type)
        {
		case spdlog::level::level_enum::debug:
		case spdlog::level::level_enum::info:
		case spdlog::level::level_enum::warn:
                break;
            case spdlog::level::level_enum::err:
				this->mAllLog->flush();
                break;
            case spdlog::level::level_enum::critical:
				this->mAllLog->flush();
                break;
            default:
                break;
        }
    }

	void LoggerComponent::CreateLogFile()
	{
		spdlog::shutdown();
        spdlog::set_level(spdlog::level::debug);
        const std::string & name = ServerConfig::Inst()->Name();
        spdlog::flush_every(std::chrono::seconds(this->mLogSaveTime));
		std::string logPath = fmt::format("{0}/{1}/{2}", this->mLogSavePath,
			Helper::Time::GetYearMonthDayString(), this->mServerName);

		spdlog::set_level(spdlog::level::level_enum::debug);
		this->mAllLog = spdlog::rotating_logger_st<spdlog::async_factory>(name,
			logPath + "/all.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
	}
}