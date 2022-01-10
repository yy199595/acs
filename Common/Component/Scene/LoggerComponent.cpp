#include"LoggerComponent.h"
#include"Core/App.h"
#include"spdlog/async.h"
#include"spdlog/sinks/stdout_color_sinks.h"
#include"spdlog/sinks/rotating_file_sink.h"
#include"Other/backward.hpp"
using namespace backward;
namespace GameKeeper
{
	// 单线程  st  多线程  mt
	bool LoggerComponent::Awake()
	{
		this->mLogSaveTime = 1;
        std::string path = "log";
		App::Get().GetConfig().GetValue("Log", "path", path);
        App::Get().GetConfig().GetValue("Log", "save", this->mLogSaveTime);
        const std::string & workPath = App::Get().GetServerPath().GetWorkPath();

        this->mLogSavePath = workPath + path;
        if (!App::Get().GetConfig().GetValue("NodeName", this->mServerName))
		{
			std::cerr << "not find config field : NodeName" << std::endl;
			return false;
		}
		this->CreateLogFile();
		return true;
	}

    bool LoggerComponent::LateAwake()
    {
        return true;
    }

	void LoggerComponent::OnZeroRefresh()
	{
		this->CreateLogFile();
	}

	void LoggerComponent::AddLog(ELogType type, const std::stringstream & stream)
	{
		switch (type)
		{
		case ELogType::info:
            this->AddInfoLog(stream);
			break;
		case ELogType::debug:
			this->AddDebugLog(stream);
			break;
		case ELogType::warn:
			this->AddWarningLog(stream);
			break;
		case ELogType::err:
			this->AddErrorLog(stream);
			break;
		case ELogType::critical:
			this->AddFatalLog(stream);
			break;
		}
	}

	void LoggerComponent::OnDestory()
	{
		spdlog::drop_all();
	}

	void LoggerComponent::CreateLogFile()
	{
		spdlog::shutdown();
		spdlog::flush_every(std::chrono::seconds(this->mLogSaveTime));
		std::string logPath = this->mLogSavePath + Helper::Time::GetYearMonthDayString();
#ifdef LOG_THREAD_LOCK
		this->mInfoLog = spdlog::rotating_logger_mt<spdlog::async_factory>("Info", logPath + "/info.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
		this->mDebugLog = spdlog::rotating_logger_mt<spdlog::async_factory>("Debug", logPath + "/debug.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
		this->mFatalLog = spdlog::rotating_logger_mt<spdlog::async_factory>("Fatal", logPath + "/fatal.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
		this->mErrorLog = spdlog::rotating_logger_mt<spdlog::async_factory>("Error", logPath + "/error.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
		this->mRecordLog = spdlog::rotating_logger_mt<spdlog::async_factory>("Record", logPath + "/record.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
		this->mWarningLog = spdlog::rotating_logger_mt<spdlog::async_factory>("Warning", logPath + "/warning.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
#else
		this->mInfoLog = spdlog::rotating_logger_st<spdlog::async_factory>("Info", logPath + "/info.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
		this->mDebugLog = spdlog::rotating_logger_st<spdlog::async_factory>("Debug", logPath + "/debug.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
		this->mFatalLog = spdlog::rotating_logger_st<spdlog::async_factory>("Fatal", logPath + "/fatal.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
		this->mErrorLog = spdlog::rotating_logger_st<spdlog::async_factory>("Error", logPath + "/error.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
		this->mRecordLog = spdlog::rotating_logger_st<spdlog::async_factory>("Record", logPath + "/record.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
		this->mWarningLog = spdlog::rotating_logger_st<spdlog::async_factory>("Warning", logPath + "/warning.log", LOG_FILE_MAX_SIZE, LOG_FILE_MAX_SUM);
#endif // LOG_THREAD_LOCK	
	}

	void LoggerComponent::AddInfoLog(const std::stringstream & stream)
	{
#ifdef _WIN32
		std::string log = stream.str();
		std::string time = Helper::Time::GetDateString();
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
			FOREGROUND_BLUE | FOREGROUND_GREEN |
			FOREGROUND_INTENSITY);
		printf("%s [Info  ] %s\n", time.c_str(), log.c_str());
#else
		std::string log = stream.str();
		std::string time = Helper::Time::GetDateString();
		printf("%s%s [Info  ] %s\e[34m\n", "\e[1m", time.c_str(), log.c_str());
#endif
		if (this->mInfoLog)
		{
			this->mInfoLog->info(stream.str());
		}		
	}

	void LoggerComponent::AddErrorLog(const std::stringstream & stream)
	{
		
#ifdef _WIN32
		std::string log = stream.str();
		std::string time = Helper::Time::GetDateString();
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
			FOREGROUND_INTENSITY | 4);
		printf("%s [Error ] %s\n", time.c_str(), log.c_str());
#else
		std::string log = stream.str();
		std::string time = Helper::Time::GetDateString();
		printf("%s%s [Error ] %s\e[0m\n", "\e[31m", time.c_str(), log.c_str());
#endif

		if (this->mErrorLog)
		{
			this->mErrorLog->error(stream.str());
		}
//        StackTrace st;
//        st.load_here(64);
//        Printer p;
//        p.color_mode = ColorMode::always;
//        p.print(st, stderr);
	}

	void LoggerComponent::AddDebugLog(const std::stringstream & stream)
	{
		
#ifdef _WIN32
		std::string log = stream.str();
		std::string time = Helper::Time::GetDateString();
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
			FOREGROUND_INTENSITY | 2);
		printf("%s [Debug ] %s\n", time.c_str(), log.c_str());
#else
		std::string log = stream.str();
		std::string time = Helper::Time::GetDateString();
		printf("%s%s [Debug ] %s\e[0m\n", "\e[32m", time.c_str(), log.c_str());
#endif
		if (this->mDebugLog)
		{
			this->mDebugLog->debug(stream.str());
		}
	}

	void LoggerComponent::AddFatalLog(const std::stringstream & stream)
	{		
#ifdef _WIN32
		std::string log = stream.str();
		std::string time = Helper::Time::GetDateString();
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
			FOREGROUND_INTENSITY | FOREGROUND_BLUE |
			FOREGROUND_RED);
		printf("%s [Fatal ] %s\n", time.c_str(), log.c_str());
#else
		std::string log = stream.str();
		std::string time = Helper::Time::GetDateString();
		printf("%s%s [Fatal ] %s\e[0m\n", "\e[35m", time.c_str(), log.c_str());
#endif
		if (this->mFatalLog)
		{
			mFatalLog->critical(stream.str());
		}
        StackTrace st;
        st.load_here(64);
        Printer p;
        p.color_mode = ColorMode::always;
        p.print(st, stderr);
	}

	void LoggerComponent::AddWarningLog(const std::stringstream & stream)
	{		
		
#ifdef _WIN32
		std::string log = stream.str();
		std::string time = Helper::Time::GetDateString();
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
			FOREGROUND_INTENSITY | 6);
		printf("%s [Warning] %s\n", time.c_str(), log.c_str());
#else
		std::string log = stream.str();
		std::string time = Helper::Time::GetDateString();
		printf("%s%s [Warning] %s\e[0m\n", "\e[33m", time.c_str(), log.c_str());
#endif
		if (this->mWarningLog)
		{
			mWarningLog->warn(stream.str());
		}
	}
}