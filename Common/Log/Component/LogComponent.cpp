#include"LogComponent.h"
#include"App/App.h"
#include"spdlog/async.h"
#ifdef __DEBUG__
#include"Log/Debug.h"
#endif
#include"System/System.h"
#include"spdlog/sinks/rotating_file_sink.h"
#include"Component/WatchDogComponent.h"
namespace Sentry
{
	// 单线程  st  多线程  mt
	bool LogComponent::Awake()
	{
		this->mLastTime = 0;
		this->mLogSaveTime = 3;
        this->mServerName = ServerConfig::Inst()->Name();
        this->mLogSavePath = fmt::format("{0}/log", System::WorkPath());
        return true;
	}

	void LogComponent::OnSecondUpdate(const int tick)
	{
		long long now = Helper::Time::NowSecTime();
		if(!Helper::Time::IsSameDay(now, this->mLastTime))
		{
			auto iter = this->mLoggers.begin();
			for(;iter != this->mLoggers.end(); iter++)
			{
				iter->second->flush();
				spdlog::drop(iter->first);
			}
			this->mLoggers.clear();
		}
		this->mLastTime = now;
	}

	void LogComponent::OnDestroy()
	{
		this->SaveAllLog();
	}

	void LogComponent::SaveAllLog()
	{
		auto iter = this->mLoggers.begin();
		for(; iter != this->mLoggers.end(); iter++)
		{
			iter->second->flush();
		}
		spdlog::drop_all();
	}

	void LogComponent::SaveLog(spdlog::level::level_enum type, const std::string& log)
    {
		const std::string & name = ServerConfig::Inst()->Name();
		std::shared_ptr<spdlog::logger> logger = this->GetLogger(name);
		if(logger != nullptr)
		{
			logger->log(type, log);
			switch (type)
			{
			case spdlog::level::level_enum::debug:
			case spdlog::level::level_enum::info:
			case spdlog::level::level_enum::warn:
				break;
			case spdlog::level::level_enum::err:
				logger->flush();
				break;
			case spdlog::level::level_enum::critical:
				logger->flush();
				break;
			default:
				break;
			}
		}
    }

	void LogComponent::SaveLog(const std::string& name,
		spdlog::level::level_enum type, const std::string& log)
	{
		std::shared_ptr<spdlog::logger> logger = this->GetLogger(name);
		if(logger == nullptr)
		{
			return;
		}
		logger->log(type, log);
	}

	std::shared_ptr<spdlog::logger> LogComponent::GetLogger(const std::string& name)
	{
		auto iter = this->mLoggers.find(name);
		if(iter == this->mLoggers.end())
		{
			try
			{
				spdlog::flush_every(std::chrono::seconds(this->mLogSaveTime));
				std::string logPath = fmt::format("{0}/{1}/{2}", this->mLogSavePath,
					Helper::Time::GetYearMonthDayString(), this->mServerName);

				const std::string path = fmt::format("{0}/{1}.log", logPath, name);

				std::shared_ptr<spdlog::logger> logger =
					spdlog::rotating_logger_st<spdlog::async_factory>(name,
						path, LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
				this->mLoggers.emplace(name, logger);
				return logger;
			}
			catch(spdlog::spdlog_ex & ex)
			{
				return nullptr;
			}
		}
		return iter->second;
	}
}