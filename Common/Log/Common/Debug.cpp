//
// Created by zmhy0073 on 2022/9/22.
//
#include"Debug.h"
#ifndef __OS_WIN__
#include<cxxabi.h>
#include<execinfo.h>
#endif
#include"Util/Time/TimeHelper.h"
#include"Entity/Unit/App.h"
#include"Log/Component/LogComponent.h"
using namespace Tendo;

#define DUMP_STACK_DEPTH_MAX 100
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
                if(Debug::Backtrace(trace) > 0)
				{
					Debug::Console(color, log);
					Debug::Console(color, trace);
					logComponent->SaveLog(color, log);
					logComponent->SaveLog(color, trace);
				}
				else
				{
					Debug::Console(color, log);
					logComponent->SaveLog(color, log);
				}
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

#ifndef __OS_WIN__
void demangle(char * msg, std::string &out)
{
	char *mangled_name = 0, *offset_begin = 0, *offset_end = 0;

	// find parantheses and +address offset surrounding mangled name
	for (char *p = msg; p && *p; ++p)
	{
		if (*p == '(')
		{
			mangled_name = p;
		}
		else if (*p == '+')
		{
			offset_begin = p;
		}
		else if (*p == ')')
		{
			offset_end = p;
			break;
		}
	}
	// if the line could be processed, attempt to demangle the symbol
	if (mangled_name && offset_begin && offset_end &&
		mangled_name < offset_begin)
	{
		*mangled_name++ = '\0';
		*offset_begin++ = '\0';
		*offset_end++ = '\0';

		int status;
		char * real_name = abi::__cxa_demangle(mangled_name, 0, 0, &status);

		// if demangling is successful, output the demangled function name
		if (status == 0)
		{
			out = out + real_name + "+" + offset_begin + offset_end;
		}
			// otherwise, output the mangled function name
		else
		{
			out = out + mangled_name + "+" + offset_begin + offset_end;
		}
		free(real_name);
	}
	else
		// otherwise, save the whole line
		out += msg;
}
#endif
int Debug::Backtrace(std::string &trace)
{
	int count = 0;
#ifdef __OS_LINUX__
	void *stack_trace[DUMP_STACK_DEPTH_MAX] = { 0 };
	char **stack_strings = NULL;
	int stack_depth = 0;
	int i = 0;
	char index[5];

	/* 获取栈中各层调用函数地址 */
	stack_depth = backtrace(stack_trace, DUMP_STACK_DEPTH_MAX);

	/* 查找符号表将函数调用地址转换为函数名称 */
	stack_strings = (char **)backtrace_symbols(stack_trace, stack_depth);
	if (NULL == stack_strings) {
		trace += " Memory is not enough while dump Stack Trace! \n";
		return 0;
	}
	/* 保存调用栈 */
	trace += "Backtrace:\n\t";
	for (i = 1; i < stack_depth; ++i)
	{
		count++;
		snprintf(index, sizeof(index), "#%02d ", i);
		trace += index;
		demangle(stack_strings[i], trace);
		trace += "\n\t";
	}

	/* 获取函数名称时申请的内存需要自行释放 */
	free(stack_strings);
	stack_strings = NULL;
#endif
	return count;
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
#ifdef __DEBUG_STACK__
            backward::StackTrace st;
            st.load_here(64);
            backward::Printer p;
            p.color_mode = backward::ColorMode::always;
            p.print(st, stderr);
#endif
        }
            break;
        default:
            break;
    }
}

void Debug::Print(Debug::Level color, const std::string & log)
{
	switch (color)
	{
		case Debug::Level::info:
		{
#ifdef _WIN32
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
				FOREGROUND_BLUE | FOREGROUND_GREEN |
				FOREGROUND_INTENSITY);
			printf("%s\n", log.c_str());
#else
			printf("%s%s\e[34m\n", "\e[1m", log.c_str());
#endif
		}
			break;
		case Debug::Level::debug:
		{
#ifdef _WIN32
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
				FOREGROUND_INTENSITY | 2);
			printf("%s\n", log.c_str());
#else
			printf("%s%s\e[0m\n", "\e[32m", log.c_str());
#endif
		}
			break;
		case Debug::Level::warn:
		{
#ifdef _WIN32
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
				FOREGROUND_INTENSITY | 6);
			printf("%s\n", log.c_str());
#else
			printf("%s%s\e[0m\n", "\e[33m", log.c_str());
#endif
		}
			break;
		case Debug::Level::err:
		{
#ifdef _WIN32
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
				FOREGROUND_INTENSITY | 4);
			printf("%s\n", log.c_str());
#else
			printf("%s%s\e[0m\n", "\e[31m", log.c_str());
#endif
		}
			break;
		case Debug::Level::critical:
		{
#ifdef _WIN32
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
				FOREGROUND_INTENSITY | FOREGROUND_BLUE |
				FOREGROUND_RED);
			printf("%s\n", log.c_str());
#else
			printf("%s%s\e[0m\n", "\e[35m", log.c_str());
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
        default:
            break;
	}
}
