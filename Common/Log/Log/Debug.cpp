//
// Created by zmhy0073 on 2022/9/22.
//
#include"Debug.h"
#include"Time/TimeHelper.h"
#include"Debug/backward.hpp"

#include"App/App.h"
#include"Component/LoggerComponent.h"
using namespace Sentry;

void Debug::Log(Debug::Level color, const std::string &log)
{
    LoggerComponent *logComponent = App::Get()->GetLogger();
    if (logComponent != nullptr)
    {
        logComponent->AddLog(color, log);
    }
}

void Debug::Console(Debug::Level color, const std::string &log)
{
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
            backward::StackTrace st;
            st.load_here(64);
            backward::Printer p;
            p.color_mode = backward::ColorMode::always;
            p.print(st, stderr);
        }
            break;
    }
}