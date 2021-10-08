#include "LogHelper.h"
#include <Util/TimeHelper.h>
#include <Util/DirectoryHelper.h>
#include <spdlog/sinks/daily_file_sink.h>

namespace Sentry
{
    std::shared_ptr<spdlog::logger> LogHelper::mErrorLog;
    std::shared_ptr<spdlog::logger> LogHelper::mFatalLog;
    std::shared_ptr<spdlog::logger> LogHelper::mWarningLog;
    std::shared_ptr<spdlog::logger> LogHelper::mRecordLog;

    void LogHelper::AddLog(ELogType type, const std::string &log) {
        std::string time = TimeHelper::GetDateString();
        switch (type) {
            case ELogType::err:
#ifdef _WIN32
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                                        FOREGROUND_INTENSITY | 4);
                printf("%s [Error ] %s\n", time.c_str(), log.c_str());
#else
                printf("%s%s [Error ] %s\e[0m\n", "\e[31m", time.c_str(), log.c_str());
#endif
                mErrorLog->error(log);
                break;
            case ELogType::debug:
#ifdef _WIN32
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                                        FOREGROUND_INTENSITY | 2);
                printf("%s [Debug ] %s\n", time.c_str(), log.c_str());
#else
                printf("%s%s [Debug ] %s\e[0m\n", "\e[32m", time.c_str(), log.c_str());
#endif
                break;
            case ELogType::warn:
#ifdef _WIN32

                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                                        FOREGROUND_INTENSITY | 6);
                printf("%s [Warning] %s\n", time.c_str(), log.c_str());
#else
                printf("%s%s [Warning] %s\e[0m\n", "\e[33m", time.c_str(), log.c_str());
#endif
                mWarningLog->warn(log);
                break;
            case ELogType::critical:
#ifdef _WIN32
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                                        FOREGROUND_INTENSITY | FOREGROUND_BLUE |
                                            FOREGROUND_RED);
                printf("%s [Fatal ] %s\n", time.c_str(), log.c_str());
#else
                printf("%s%s [Fatal ] %s\e[0m\n", "\e[35m", time.c_str(), log.c_str());
#endif
                mFatalLog->critical(log);
                break;
            case ELogType::info:
#ifdef _WIN32
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                                        FOREGROUND_BLUE | FOREGROUND_GREEN |
                                            FOREGROUND_INTENSITY);
                printf("%s [Info  ] %s\n", time.c_str(), log.c_str());
#else
                printf("%s%s [Info  ] %s\e[34m\n", "\e[1m", time.c_str(), log.c_str());
#endif
                break;
        }
    }

    bool LogHelper::Init(const std::string path, const std::string name)
    {
        std::string logPath = path + "/" + TimeHelper::GetYearMonthDayString();
        mFatalLog = spdlog::basic_logger_mt(name + ".fatal", logPath + "/fatal.log");
        mErrorLog = spdlog::basic_logger_mt(name + ".error", logPath + "/error.log");
        mRecordLog = spdlog::basic_logger_mt(name + ".record", logPath + "/record.log");
        mWarningLog = spdlog::basic_logger_mt(name + ".warning", logPath + "/warning.log");

        spdlog::flush_every(std::chrono::seconds(1));

        return true;
    }

}// namespace Sentry