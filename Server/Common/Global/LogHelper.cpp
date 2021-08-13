#include "LogHelper.h"
#include <Util/TimeHelper.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace Sentry
{
    void LogInfoConsole::ShowMessage(const std::string &log)
    {
#ifdef _WIN32
        std::string time = TimeHelper::GetDateString();
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                                FOREGROUND_BLUE | FOREGROUND_GREEN |
                                    FOREGROUND_INTENSITY);
        printf("%s [Info  ] %s\n", time.c_str(), log.c_str());
#else
        std::string time = TimeHelper::GetDateString();
        printf("%s%s [Info  ] %s\e[34m\n", "\e[1m", time.c_str(), log.c_str());
#endif
    }

    void LogDebugConsole::ShowMessage(const std::string &log)
    {
#ifdef _WIN32
        std::string time = TimeHelper::GetDateString();
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                                FOREGROUND_INTENSITY | 2);
        printf("%s [Debug ] %s\n", time.c_str(), log.c_str());
#else
        std::string time = TimeHelper::GetDateString();
        printf("%s%s [Debug ] %s\e[0m\n", "\e[32m", time.c_str(), log.c_str());
#endif
    }

    void LogErrorConsole::ShowMessage(const std::string &log)
    {
#ifdef _WIN32
        std::string time = TimeHelper::GetDateString();
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                                FOREGROUND_INTENSITY | 4);
        printf("%s [Error ] %s\n", time.c_str(), log.c_str());
#else
        std::string time = TimeHelper::GetDateString();
        printf("%s%s [Error ] %s\e[0m\n", "\e[31m", time.c_str(), log.c_str());
#endif
    }

    void LogFatalConsole::ShowMessage(const std::string &log)
    {
#ifdef _WIN32
        std::string time = TimeHelper::GetDateString();
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                                FOREGROUND_INTENSITY | FOREGROUND_BLUE |
                                    FOREGROUND_RED);
        printf("%s [Fatal ] %s\n", time.c_str(), log.c_str());
#else
        std::string time = TimeHelper::GetDateString();
        printf("%s%s [Fatal ] %s\e[0m\n", "\e[35m", time.c_str(), log.c_str());
#endif
    }

    void LogWarningConsole::ShowMessage(const std::string &log)
    {
#ifdef _WIN32
        std::string time = TimeHelper::GetDateString();
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                                FOREGROUND_INTENSITY | 6);
        printf("%s [Warning] %s\n", time.c_str(), log.c_str());
#else
        std::string time = TimeHelper::GetDateString();
        printf("%s%s [Warning] %s\e[0m\n", "\e[33m", time.c_str(), log.c_str());
#endif
    }
} // namespace Sentry

namespace Sentry
{
    void LogHelper::PushLog(ELogType type, const std::string &log)
    {
        auto iter = mLoggerMap.find(type);
        if (iter != mLoggerMap.end() && iter->second != nullptr)
        {
            iter->second->ShowMessage(log);
            this->mLogger->flush();
        }
    }

    LogHelper *LogHelper::mLogHelper = nullptr;

    void LogHelper::AddLog(ELogType type, const std::string &log)
    {
        auto iter = mLogHelper->mLoggerMap.find(type);
        if (iter != mLogHelper->mLoggerMap.end())
        {
            iter->second->ShowMessage(log);

            mLogHelper->mLogger->log((spdlog::level::level_enum) type, log);
            mLogHelper->mLogger->flush();
        }
    }

    LogHelper::LogHelper(const std::string path, const std::string name)
    {
        if (mLogHelper != nullptr)
        {
            throw;
        }
        mLogHelper = this;
        std::stringstream pathStream;
        this->mLogPath =
                path + "/" + name + "_" + TimeHelper::GetYearMonthDayString();
        this->mLogger =
                spdlog::rotating_logger_mt(name, this->mLogPath + ".log", 100000, 100);

        this->mLoggerMap.insert(std::make_pair(ELogType::info, new LogInfoConsole()));
        this->mLoggerMap.insert(std::make_pair(ELogType::err, new LogErrorConsole()));
        this->mLoggerMap.insert(
                std::make_pair(ELogType::critical, new LogFatalConsole()));
        this->mLoggerMap.insert(
                std::make_pair(ELogType::debug, new LogDebugConsole()));
        this->mLoggerMap.insert(
                std::make_pair(ELogType::warn, new LogWarningConsole()));
    }
} // namespace Sentry