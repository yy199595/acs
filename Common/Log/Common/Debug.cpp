//
// Created by zmhy0073 on 2022/9/22.
//
#include"Debug.h"
#include "Rang.h"
#ifdef __OS_LINUX__
#include<execinfo.h>
#endif

#ifndef __OS_WIN__
#include<cxxabi.h>
#else
#include <windows.h>
#include <dbghelp.h>
#include "Util/File/FileHelper.h"

#endif

#include"Util/Tools/TimeHelper.h"
#include"Entity/Actor/App.h"
#include"Log/Component/LoggerComponent.h"


#include"XCode/XCode.h"
#include "Server/Config/CodeConfig.h"

using namespace acs;

#define DUMP_STACK_DEPTH_MAX 100

void Debug::LuaError(const char* str)
{
	//const char * log = strstr(str, ".lua");
	//Debug::Log(custom::LogLevel::Error, str);
}

#ifdef __OS_WIN__

bool Debug::Init()
{
	return SymInitialize(GetCurrentProcess(), nullptr, TRUE);
}

#endif

void Debug::Log(std::unique_ptr<custom::LogInfo> log)
{
	static LoggerComponent* logComponent = nullptr;
	if (logComponent == nullptr)
	{
		logComponent = App::Get<LoggerComponent>();
		if (logComponent == nullptr)
		{
			return;
		}
	}
	if (log->Level >= custom::LogLevel::Fatal)
	{
		Debug::Backtrace(log->Stack);
	}
	logComponent->PushLog(std::move(log));
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

int Debug::Backtrace(std::string& trace)
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
#elif __OS_WIN__
	HANDLE process = GetCurrentProcess();
	HANDLE thread = GetCurrentThread();

	CONTEXT context = {};
	context.ContextFlags = CONTEXT_FULL;
	RtlCaptureContext(&context);

	STACKFRAME64 stackFrame;
	memset(&stackFrame, 0, sizeof(STACKFRAME64));

#ifdef _M_IX86
	DWORD machineType = IMAGE_FILE_MACHINE_I386;
	stackFrame.AddrPC.Offset = context.Eip;
	stackFrame.AddrFrame.Offset = context.Ebp;
	stackFrame.AddrStack.Offset = context.Esp;
#elif _M_X64
	DWORD machineType = IMAGE_FILE_MACHINE_AMD64;
	stackFrame.AddrPC.Offset = context.Rip;
	stackFrame.AddrFrame.Offset = context.Rbp;
	stackFrame.AddrStack.Offset = context.Rsp;
#endif

	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrStack.Mode = AddrModeFlat;

	for (int i = 0; i < 20; i++)
	{
		if (!StackWalk64(machineType, process, thread, &stackFrame, &context, nullptr, SymFunctionTableAccess64,
				SymGetModuleBase64, NULL))
		{
			break;
		}

		if (stackFrame.AddrPC.Offset == 0)
		{
			break;
		}
		// 获取符号
		DWORD64 address = stackFrame.AddrPC.Offset;
		char symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
		SYMBOL_INFO* symbol = (SYMBOL_INFO*)symbolBuffer;
		symbol->MaxNameLen = MAX_SYM_NAME;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

		DWORD64 displacement = 0;
		if (SymFromAddr(process, address, &displacement, symbol) && i >= 2)
		{
			IMAGEHLP_LINE64 line;
			DWORD displacementLine = 0;
			memset(&line, 0, sizeof(line));
			line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
			if (SymGetLineFromAddr64(process, address, &displacementLine, &line))
			{
				std::string fileName(line.FileName);
				size_t pos = fileName.find_last_of('\\');
				if (pos != std::string::npos)
				{
					fileName = fileName.substr(pos + 1);
				}
				count++;
				trace.append(fmt::format("#{}: {} {:#x} {}:{}\n", i - 2, symbol->Name, symbol->Address, fileName,
						line.LineNumber));
			}
		}
	}
#endif
	return count;
}

void Debug::Console(custom::LogLevel level, int code)
{
	if (code == XCode::Ok)
		return;
	custom::LogInfo logInfo;
	{
		logInfo.Level = level;
		const std::string& desc = acs::CodeConfig::Inst()->GetDesc(code);
		logInfo.Content = fmt::format("code = {}", desc);
	}
	Debug::Console(logInfo);
}

void Debug::Console(const custom::LogInfo& logInfo)
{
	const static std::string empty(" ");
	const std::string& file = logInfo.File;
	const std::string& log = logInfo.Content;
	std::string time = help::Time::GetDateString();

	switch (logInfo.Level)
	{
	case custom::LogLevel::Info:
		std::cout << rang::fg::cyan << time << " [Info   ] " << file << empty << log << std::endl;
		break;
	case custom::LogLevel::Debug:
		std::cout << rang::fg::green << time << " [Debug  ] " << file << empty << log << std::endl;
		break;
	case custom::LogLevel::Warn:
		std::cout << rang::fg::yellow << time << " [Warning] " << file << empty << log << std::endl;
		break;
	case custom::LogLevel::Error:
		std::cout << rang::fg::red << time << " [Error  ] " << file << empty << log << std::endl;
		break;
	case custom::LogLevel::Fatal:
		std::cout << rang::fg::magenta << time << " [Fatal  ] " << file << empty << log << std::endl;
		break;
	default:
		break;
	}
	if (!logInfo.Stack.empty())
	{
		std::cout << rang::fgB::magenta << logInfo.Stack << std::endl;
	}
}

void Debug::Print(custom::LogLevel level, const std::string& log)
{
	switch (level)
	{
	case custom::LogLevel::Info:
		std::cout << rang::fgB::cyan << log << std::endl;
		break;
	case custom::LogLevel::Debug:
		std::cout << rang::fgB::green << log << std::endl;
		break;
	case custom::LogLevel::Warn:
		std::cout << rang::fgB::yellow << log << std::endl;
		break;
	case custom::LogLevel::Error:
		std::cout << rang::fgB::red << log << std::endl;
		break;
	case custom::LogLevel::Fatal:
		std::cout << rang::fgB::magenta << log << std::endl;
		break;
	default:
		break;
	}
}
