//
// Created by zmhy0073 on 2022/10/13.
//

#include"System.h"
#ifdef __OS_WIN__
#include<direct.h>
#include<Windows.h>
#include <psapi.h>
#include<iostream>
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "kernel32.lib")
#endif

#ifdef __OS_MAC__
#include<unistd.h>
#include<mach/mach.h>
#include<sys/sysctl.h>
#endif

#ifdef __OS_LINUX__
#include<unistd.h>
#include<fstream>
#include<cstdint>
#include<sys/sysinfo.h>
#include<sys/resource.h>
//#include<memory_resource>
#endif

#include<cstdlib>
#include<fstream>
#include"fmt.h"
#include"Util/Tools/String.h"
#include"Log/Common/CommonLogDef.h"

#ifdef __OS_WIN__

#include <dbghelp.h>
#include <tchar.h>
#include <Util/Tools/TimeHelper.h>
#pragma comment(lib, "dbghelp.lib")

namespace os
{
	bool CreateMiniDump(EXCEPTION_POINTERS* pep) {
		// 打开文件
		std::string name = fmt::format("{}.dmp", help::Time::GetDateStr());
		HANDLE hFile = CreateFile(_T(name.c_str()), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			return false;
		}

		// 创建转储文件
		MINIDUMP_EXCEPTION_INFORMATION mei;
		mei.ThreadId = GetCurrentThreadId();
		mei.ExceptionPointers = pep;
		mei.ClientPointers = FALSE;

		BOOL result = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &mei, NULL, NULL);
		CloseHandle(hFile);
		return result != 0;
	}

	LONG WINAPI MyUnhandledExceptionFilter(EXCEPTION_POINTERS* pExceptionPtrs) {
		CreateMiniDump(pExceptionPtrs);
		return EXCEPTION_EXECUTE_HANDLER;
	}
}
#endif

namespace os
{
	bool System::Init(int argc, char** argv)
	{
		std::vector<std::string> commandLine;
		commandLine.reserve(argc);
        for(int index = 0; index < argc; index++)
		{
			commandLine.emplace_back(argv[index]);
		}
		System::SetEnv("id", "0"); //服务器id
		System::SetEnv("log", "1"); //日志等级
		System::SetEnv("name", "server"); //集群名称
		System::SetEnv("db", "127.0.0.1"); //db地址
		System::SetEnv("exe",  commandLine[0]);
		System::SetEnv("config", "./config/run/all.json");
		std::string work = fmt::format("{0}/", getcwd(nullptr, 0));
		help::Str::ReplaceString(work, "\\", "/");
		if (work.back() == '/')
		{
			work.pop_back();
		}
		System::mWorkPath = work;
		std::vector<std::string> result;
		for (size_t index = 1; index < commandLine.size(); index++)
		{
			result.clear();
			std::string line(commandLine[index]);
			if(line.find("--") == std::string::npos)
			{
				CONSOLE_LOG_ERROR("args:{} format error", line);
				return false;
			}
			const std::string str = line = line.substr(2);
			if (help::Str::Split(str, '=', result) != 2)
			{
				CONSOLE_LOG_ERROR("args:{} format error", line);
				return false;
			}
			System::SetEnv(result[0], result[1]);
		}
		std::string path;
		System::GetEnv("CONFIG", path);
		CONSOLE_LOG_DEBUG("config path = {}", path);
#ifdef __OS_WIN__
		SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
#endif
		return System::AddValue("WORK_DIR", work);
	}

	bool System::AddValue(const std::string& key, const std::string& value)
	{
		char buffer[256] = { 0 };
		sprintf(buffer, "${%s}", key.c_str());
		mSubValues[std::string(buffer)] = value;
		return true;
	}

	void System::LuaSetEnv(const char* key, const char* val)
	{
		System::SetEnv(key, val);
	}

	bool System::GetEnv(const std::string& k, int& v)
	{
		std::string value;
		if (!System::GetEnv(k, value))
		{
			return false;
		}
		v = std::stoi(value);
		return true;
	}

	bool System::GetEnv(const std::string& k, std::string& v)
	{
		std::string str = fmt::format("APP_{}", k);
		const std::string key = help::Str::Toupper(str);
		const char * val = getenv(key.c_str());
		if(val == nullptr)
		{
			return false;
		}
		v = val;
		return true;
	}

	bool System::SubValue(std::string& value)
	{
		std::smatch match;
		std::regex re(R"(\$?\{[^}]*\})");   // 匹配 ${...} 或 $${...} 格式的字符串
		if (std::regex_search(value, match, re))
		{
			auto iter = mSubValues.begin();
			for (; iter != mSubValues.end(); iter++)
			{
				const std::string& key = iter->first;
				const std::string& val = iter->second;
				help::Str::ReplaceString(value, key, val);
			}
			return true;
		}
		return false;
	}

	bool System::HasEnv(const std::string& k)
	{
		return getenv(k.c_str()) != nullptr;
	}

	void System::SetEnv(const std::string& k, const std::string& v)
	{
		std::string str = fmt::format("APP_{}", k);
		const std::string key = help::Str::Toupper(str);
#if WIN32
		_putenv_s(key.c_str(), v.c_str());
#else
		setenv(key.c_str(), v.c_str(), 1);
#endif
		System::AddValue(k, v);
	}

	bool System::ReadFile(const std::string& path, std::string& content)
	{
		std::ifstream fs(path);
		if(!fs.is_open())
		{
			return false;
		}
		std::string lineData;
		while(std::getline(fs, lineData))
		{
			System::SubValue(lineData);
			content.append(lineData);
			content += "\n";
		}
		return true;
	}

    std::string System::FormatPath(const std::string &path)
    {
		if(path[0] == '/')
		{
			return System::mWorkPath + path;
		}
		return fmt::format("{}/{}", mWorkPath, path);
    }

	bool System::GetSystemInfo(SystemInfo& systemInfo)
	{
#ifdef __OS_MAC__
		mach_task_basic_info_data_t info;
		mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
		task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &count);
		systemInfo.use_memory = info.resident_size;

		int mib[2];
		size_t len;
		mib[0] = CTL_HW;
		mib[1] = HW_MEMSIZE;
		uint64_t physicalMemorySize = 0;
		len = sizeof(physicalMemorySize);
		sysctl(mib, 2, &physicalMemorySize, &len, NULL, 0);
		systemInfo.max_memory = physicalMemorySize;

		thread_basic_info_data_t threadInfo;
		mach_msg_type_number_t threadInfoCount = THREAD_BASIC_INFO_COUNT;
		thread_info(mach_thread_self(), THREAD_BASIC_INFO, reinterpret_cast<thread_info_t>(&threadInfo), &threadInfoCount);
		unsigned int cpuUsage = threadInfo.cpu_usage;
		// 获取系统核数
		host_basic_info_data_t hostInfo{};
		mach_msg_type_number_t hostInfoCount = HOST_BASIC_INFO_COUNT;
		host_info(mach_host_self(), HOST_BASIC_INFO, reinterpret_cast<host_info_t>(&hostInfo), &hostInfoCount);
		unsigned int processorCount = hostInfo.max_cpus;
		// 计算 CPU 使用率
		systemInfo.cpu = static_cast<double>(cpuUsage) / (TH_USAGE_SCALE * processorCount) * 100.0;
#elif __OS_LINUX__
		struct rusage r_usage;
		getrusage(RUSAGE_SELF, &r_usage);
		constexpr double mb = 1024.f * 1024;

		// 计算CPU使用率
		double cpu_usage = (double)r_usage.ru_utime.tv_sec + (double)r_usage.ru_utime.tv_usec / 1e6;
		double total_cpu_time = (double)r_usage.ru_stime.tv_sec + (double)r_usage.ru_stime.tv_usec / 1e6;
		systemInfo.cpu = cpu_usage / total_cpu_time;
		//systemInfo.use_memory = r_usage.ru_maxrss;

		std::ifstream statm("/proc/self/statm");
		if (!statm.is_open()) {
			// 打开文件失败
			return false;
		}
		std::string line;
		std::getline(statm, line);
		statm.close();
		std::size_t total_pages, resident_pages, shared_pages;
		std::sscanf(line.c_str(), "%zu %zu %zu", &total_pages, &resident_pages, &shared_pages);
		std::size_t page_size = sysconf(_SC_PAGESIZE);
		systemInfo.use_memory = (resident_pages * page_size);

		struct sysinfo mem_info;
    	if(sysinfo(&mem_info) != 0) {
        	return false;
    	}
		systemInfo.max_memory = mem_info.totalram;
#else
		HANDLE hProcess = GetCurrentProcess();
		FILETIME ftSysIdle, ftSysKernel, ftSysUser, ftProcCreation, ftProcExit, ftProcKernel, ftProcUser;
		if (GetSystemTimes(&ftSysIdle, &ftSysKernel, &ftSysUser) && GetProcessTimes(hProcess, &ftProcCreation, &ftProcExit, &ftProcKernel, &ftProcUser))
		{
			ULONGLONG sysKernel = static_cast<ULONGLONG>(ftSysKernel.dwHighDateTime) << 32 | ftSysKernel.dwLowDateTime;
			ULONGLONG sysUser = static_cast<ULONGLONG>(ftSysUser.dwHighDateTime) << 32 | ftSysUser.dwLowDateTime;
			ULONGLONG procKernel = static_cast<ULONGLONG>(ftProcKernel.dwHighDateTime) << 32 | ftProcKernel.dwLowDateTime;
			ULONGLONG procUser = static_cast<ULONGLONG>(ftProcUser.dwHighDateTime) << 32 | ftProcUser.dwLowDateTime;
			ULONGLONG sysTime = sysKernel + sysUser;
			ULONGLONG procTime = procKernel + procUser;
			systemInfo.cpu = procTime / (double)sysTime;
		}

		PROCESS_MEMORY_COUNTERS pmc;
		GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc));
		systemInfo.use_memory = (long long)pmc.WorkingSetSize;

		MEMORYSTATUSEX memStatus;
		memStatus.dwLength = sizeof(memStatus);
		GlobalMemoryStatusEx(&memStatus);
		systemInfo.max_memory = (long long)memStatus.ullTotalPhys;
        CloseHandle(hProcess);
#endif
		return true;
	}

    std::string System::mWorkPath;
	std::unordered_map<std::string, std::string> System::mSubValues;
}