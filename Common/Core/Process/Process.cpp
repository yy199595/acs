//
// Created by yy on 2024/7/5.
//

#include "Process.h"
#include <iostream>
#ifdef __OS_WIN__
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#else

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <csignal>

#endif
#include "Core/System/System.h"
#include "Util/File/DirectoryHelper.h"

namespace os
{
	Process::Process(const std::string& exe, const std::string & name): 
		mPid(0), mExe(exe), mName(name)
	{
#ifdef __OS_WIN__
		this->hProcess = nullptr;
#endif
		std::string tmp;
		help::dir::GetDirAndFileName(exe, this->mDir, tmp);
	}

	bool Process::Close()
	{
#ifdef __OS_WIN__
		HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, this->mPid);
		if (hProcess == NULL) {
			return false;
		}

		if (!TerminateProcess(hProcess, 0)) {
			CloseHandle(hProcess);
			return false;
		}
		//WaitForSingleObject(hProcess, INFINITE);

		CloseHandle(hProcess);
		return true;
#else
		return kill(this->mPid, SIGTERM) == 0;
#endif
	}

	bool Process::IsRunning() const
	{
#ifdef _WIN32
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, this->mPid);
		if (hProcess == NULL) {
			return false;
		}

		DWORD exitCode = 0;
		if (GetExitCodeProcess(hProcess, &exitCode) == 0) {
			CloseHandle(hProcess);
			return false;
		}

		CloseHandle(hProcess);
		return (exitCode == STILL_ACTIVE);

#else  // Linux and macOS
		if (kill(this->mPid, 0) == 0)
		{
			// Process exists
			return true;
		}
		return false;
#endif
	}

	bool Process::Start(const std::string& args)
	{
		this->mArgs.emplace_back(args);
		return this->Run();
	}

	bool Process::Start(const std::vector<std::string>& args)
	{
		this->mArgs = args;
		return this->Run();
	}

	std::string Process::CmdLine()
	{
		std::string cmdLine = this->mExe;
		for (const std::string& a : this->mArgs)
		{
			cmdLine.append(" ");
			cmdLine.append(a);
		}
		return cmdLine;
	}

	bool Process::Run()
	{
		if (this->mExe.empty() || this->mDir.empty())
		{
			return false;
		}

		std::string cmdLine = this->CmdLine();
#ifdef __OS_WIN__
		std::string dir = os::System::WorkPath();
		if (!SetCurrentDirectory(this->mDir.c_str()))
		{
			return false;
		}
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_SHOWNORMAL;
		if (!CreateProcessA(NULL,   // No module name (use command line)
			const_cast<char*>(cmdLine.c_str()), // Command line
			NULL,   // Process handle not inheritable
			NULL,   // Thread handle not inheritable
			FALSE,  // Set handle inheritance to FALSE
			0,      // No creation flags
			NULL,   // Use parent's environment block
			NULL,   // Use parent's starting directory
			&si,    // Pointer to STARTUPINFO structure
			&pi))   // Pointer to PROCESS_INFORMATION structure
		{
			SetCurrentDirectory(dir.c_str());
			return false;
		}

		this->mPid = pi.dwProcessId;
		this->hProcess = pi.hProcess;
		SetCurrentDirectory(dir.c_str());

		return true;
#else  // Linux and macOS
		std::string dir = os::System::WorkPath();
		chdir(this->mDir.c_str());
		this->mPid = fork();

		std::cout << "pid = " << this->mPid << std::endl;
		if (this->mPid == 0)
		{
			std::vector<const char*> c_args;
			c_args.push_back(this->mExe.c_str());
			for (const std::string& a: this->mArgs)
			{
				c_args.push_back(a.c_str());
			}
			c_args.push_back(nullptr);
			execvp(this->mExe.c_str(), const_cast<char* const*>(c_args.data()));
			chdir(dir.c_str());
			return true;
		}
		chdir(dir.c_str());
		return false;
#endif
	}
}