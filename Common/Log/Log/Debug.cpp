//
// Created by zmhy0073 on 2022/9/22.
//
#include"Debug.h"
#include"Time/TimeHelper.h"
#include"App/App.h"
#ifndef __DEBUG_STACK__
#include"backward.hpp"
#endif

#include"Component/LogComponent.h"
#include"Component/WatchDogComponent.h"
using namespace Sentry;
void Debug::Lua(const char *log)
{
    std::string logMessage(log);
    size_t pos = logMessage.find_last_of('/');
    if (pos != std::string::npos)
    {
        logMessage = logMessage.substr(pos + 1);
    }
    Debug::Log(Debug::Level::err, logMessage);
}

void Debug::LuaError(const char* str)
{
	//const char * log = strstr(str, ".lua");
	Debug::Log(Debug::Level::err, str);
}

void Debug::Log(Debug::Level color, const std::string &log)
{
    LogComponent *logComponent = App::Inst()->GetLogger();
    if (logComponent != nullptr)
    {
        switch (color)
        {
            case spdlog::level::err:
            case spdlog::level::critical:
            {
                std::string trace;
                Debug::Backtrace(trace, 15, 2);
                Debug::Console(color, log + trace);
				logComponent->SaveLog(color, log + trace);
            }
                break;
            default:
            {
                Debug::Console(color, log);
				logComponent->SaveLog(color, log);
            }
                break;
        }
    }
}

void Debug::Backtrace(std::string &trace, int size, int skip)
{
    
}

void Debug::Console(Debug::Level color, const std::string &log)
{
    //Debug::ShowInWatchDog(color, log);
#ifdef __ALL_OUTPUT_LOG__
    LogComponent *logComponent = App::Inst()->GetLogger();
    if (logComponent != nullptr)
    {
        logComponent->OutputLog(color, log);
    }
#endif
    switch (color)
    {
        case Debug::Level::info:
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
            break;
        case Debug::Level::debug:
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
            break;
        case Debug::Level::warn:
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
            break;
        case Debug::Level::err:
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
            break;
        case Debug::Level::critical:
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
#ifdef __DEBUG_STACK__
            backward::StackTrace st;
            st.load_here(64);
            backward::Printer p;
            p.color_mode = backward::ColorMode::always;
            p.print(st, stderr);
#endif
        }
            break;
    }

}
