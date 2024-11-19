//
// Created by yy on 2023/8/12.
//
#include"Log/Common/Debug.h"
#include"ConsoleOutput.h"
#include "Util/Tools/TimeHelper.h"

namespace custom
{
	void ConsoleOutput::Push(Asio::Context &io, const std::string& name, const custom::LogInfo& logInfo)
	{
		const std::string & file = logInfo.File;
		const std::string & log = logInfo.Content;
		std::string time = help::Time::GetDateString();
		switch (logInfo.Level)
		{
			case LogLevel::Info:
			{
#ifdef _WIN32
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
            FOREGROUND_BLUE | FOREGROUND_GREEN |
            FOREGROUND_INTENSITY);
        printf("%s [info ] %s %s\n", time.c_str(), file.c_str(), log.c_str());
#else
				printf("%s%s [info  ] %s %s\e[34m\n", "\e[1m", time.c_str(), file.c_str(),  log.c_str());
#endif
			}
				break;
			case LogLevel::Debug:
			{
#ifdef _WIN32
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
            FOREGROUND_INTENSITY | 2);
        printf("%s [debug] %s %s\n", time.c_str(), file.c_str(), log.c_str());
#else
				printf("%s%s [debug ] %s %s\e[0m\n", "\e[32m", time.c_str(), file.c_str(), log.c_str());
#endif
			}
				break;
			case LogLevel::Warn:
			{
#ifdef _WIN32
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
            FOREGROUND_INTENSITY | 6);
        printf("%s [warn ] %s %s\n", time.c_str(), file.c_str(), log.c_str());
#else
				printf("%s%s [warn  ] %s %s\e[0m\n", "\e[33m", time.c_str(), file.c_str(), log.c_str());
#endif
			}
				break;
			case LogLevel::Error:
			{
#ifdef _WIN32
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
            FOREGROUND_INTENSITY | 4);
        printf("%s [error] %s %s\n", time.c_str(), file.c_str(), log.c_str());
#else
				printf("%s%s [error ] %s %s\e[0m\n", "\e[31m", time.c_str(), file.c_str(), log.c_str());
#endif
			}
				break;
			case LogLevel::Fatal:
			{
#ifdef _WIN32
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
            FOREGROUND_INTENSITY | FOREGROUND_BLUE |
            FOREGROUND_RED);
        	printf("%s [fatal] %s %s\n", time.c_str(), file.c_str(), log.c_str());
#else
				printf("%s%s [fatal ] %s %s\e[0m\n", "\e[35m", time.c_str(), file.c_str(), log.c_str());
#endif
			}
				break;
			default:
				break;
		}
		if(!logInfo.Stack.empty())
		{
			printf("%s\n", logInfo.Stack.c_str());
		}
	}
}