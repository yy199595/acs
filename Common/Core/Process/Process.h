//
// Created by yy on 2024/7/5.
//

#ifndef APP_PROCESS_H
#define APP_PROCESS_H
#include <string>
#include <vector>
namespace os
{
	class Process
	{
	public:
		Process(const std::string& exe, const std::string & name);
	public:
		bool Run();
		bool Close();
		bool IsRunning() const;
		bool Start(const std::string& args);
		bool Start(const std::vector<std::string>& args);
	public:
		std::string CmdLine();
		inline long long GetPID() const { return this->mPid; }
		inline const std::string& Name() const { return this->mName; }
		inline const std::string& GetExe() const { return this->mExe; }
	private:
		long long mPid;
#ifdef __OS_WIN__
		void* hProcess;
#endif
		std::string mDir;
		std::string mExe;
		std::string mName;
		std::vector<std::string> mArgs;
	};
}

#endif //APP_PROCESS_H
