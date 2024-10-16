//
// Created by zmhy0073 on 2022/9/22.
//

#ifndef APP_CONSOLE_H
#define APP_CONSOLE_H

#endif //APP_CONSOLE_H

#include<string>
#include"Level.h"
#include"fmt.h"

namespace Debug
{
#ifdef __OS_WIN__
	extern bool Init();
#endif
	extern void LuaError(const char * str);
	extern int Backtrace(std::string & trace);
	extern void Console(const custom::LogInfo & log);
	extern void Console(custom::LogLevel level, int code);
	extern void Log(std::unique_ptr<custom::LogInfo> log);
	extern void Print(custom::LogLevel level, const std::string & log);
}