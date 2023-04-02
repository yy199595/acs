//
// Created by zmhy0073 on 2022/9/22.
//

#ifndef APP_CONSOLE_H
#define APP_CONSOLE_H

#endif //APP_CONSOLE_H

#include<string>
#include"spdlog/common.h"
namespace Debug
{
    typedef spdlog::level::level_enum Level;
    extern void Lua(const char * log);
	extern void LuaError(const char * str);
    extern void Log(Debug::Level color, const std::string & log);
    extern void Backtrace(std::string & trace, int size, int skip);
    extern void Console(Debug::Level color, const std::string & log);
	extern void Print(Debug::Level color, const std::string & log);

}