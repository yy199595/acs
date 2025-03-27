//
// Created by yy on 2023/8/12.
//

#include"FileOutput.h"
#include"fmt.h"
#include"Util/Tools/TimeHelper.h"
#include"Util/File/FileHelper.h"
#include"Util/File/DirectoryHelper.h"
#include "Log/Common/CommonLogDef.h"
namespace custom
{
	FileOutput::FileOutput(FileConfig config)
		: mConfig(std::move(config))
	{
		this->mIndex = 0;
		this->mFileLine = 0;
		this->mOpenFileTime = 0;
	}

	void FileOutput::Close()
	{
		if(this->mDevStream.is_open())
		{
			this->mDevStream.flush();
			this->mDevStream.close();
		}
	}

	bool FileOutput::Init()
	{
		this->mIndex = 0;
		this->mFileLine = 0;
		const std::string & root = this->mConfig.Root;
		std::string time = help::Time::GetYearMonthDayString();
		const std::string dir = fmt::format("{}/{}", root, time);
		if(!help::dir::MakeDir(dir))
		{
			return false;
		}
		this->mOpenFileTime = help::Time::NowSec();
		this->mPath = fmt::format("{}/{}.log", dir, this->mConfig.Name);
		return true;
	}

	bool FileOutput::Start(Asio::Context& io)
	{
		return this->Init();
	}

	bool FileOutput::OpenFile()
	{
		if(!this->mDevStream.is_open())
		{
			this->mDevStream.open(this->mPath, std::ios::out | std::ios::app | std::ios::ate);
			if(!this->mDevStream.is_open())
			{
				return false;
			}
			this->mFileLine = 0;
			help::fs::GetFileLine(this->mPath, this->mFileLine);
		}
		return true;
	}

	void FileOutput::OnTick(int tick)
	{

	}

	void FileOutput::Push(Asio::Context &io, const std::string& name, const custom::LogInfo& logData)
	{
		if (!this->OpenFile())
		{
			return;
		}
		static std::string LogDebug(" [debug]");
		static std::string LogInfo(" [info ]");
		static std::string LogWarn(" [warn ]");
		static std::string LogError(" [error]");
		static std::string LogFatal(" [fatal]");
		long long nowTime = help::Time::NowSec();
		std::string time = help::Time::GetDateString(nowTime);
		if(!help::Time::IsSameDay(nowTime, this->mOpenFileTime))
		{
			this->Init();
			this->SwitchFile();
		}
		switch (logData.Level)
		{
			case LogLevel::Debug:
				this->mDevStream.write(time.c_str(), (int)time.size()) << " ";
				this->mDevStream.write(LogDebug.c_str(), (int)LogDebug.size());
				break;
			case LogLevel::Info:
				this->mDevStream.write(time.c_str(), (int)time.size()) << " ";
				this->mDevStream.write(LogInfo.c_str(), (int)LogInfo.size());
				break;
			case LogLevel::Warn:
				this->mDevStream.write(time.c_str(), (int)time.size()) << " ";
				this->mDevStream.write(LogWarn.c_str(), (int)LogWarn.size());
				break;
			case LogLevel::Error:
				this->mDevStream.write(time.c_str(), (int)time.size()) << " ";
				this->mDevStream.write(LogError.c_str(), (int)LogError.size());
				break;
			case LogLevel::Fatal:
				this->mDevStream.write(time.c_str(), (int)time.size()) << " ";
				this->mDevStream.write(LogFatal.c_str(), (int)LogFatal.size());
				break;
		}
		if (!logData.File.empty())
		{
			this->mDevStream.write(logData.File.c_str(), (int)logData.File.size());
			this->mDevStream << " ";
		}

		this->mDevStream.write(logData.Content.c_str(), (int)logData.Content.size());

		this->mDevStream << "\n";
		if (!logData.Stack.empty())
		{
			this->mFileLine++;
			this->mDevStream << logData.Stack << "\n";
		}
		this->mFileLine++;
		if(this->mFileLine >= this->mConfig.MaxLine)
		{
			this->SwitchFile();
			return;
		}
		size_t fileSize = this->mDevStream.tellp();
		if(fileSize >= this->mConfig.MaxSize)
		{
			this->SwitchFile();
			return;
		}
		this->mDevStream.flush();
	}

	void FileOutput::SwitchFile()
	{
		this->mIndex++;
		this->mFileLine = 0;
		this->mDevStream.flush();
		this->mDevStream.close();
		const std::string & root = this->mConfig.Root;
		std::string time = help::Time::GetYearMonthDayString();

		std::string dir = fmt::format("{}/{}", root, time);
		if(!help::dir::DirectorIsExist(dir))
		{
			this->mIndex = 0;
			help::dir::MakeDir(dir);
		}
		std::string path = fmt::format("{}/{}-{}.log", dir, this->mConfig.Name, this->mIndex);

		while(help::fs::FileIsExist(path))
		{
			this->mIndex++;
			path = fmt::format("{}/{}-{}.log", dir, this->mConfig.Name, this->mIndex);
		}
		std::rename(this->mPath.c_str(), path.c_str());
		this->OpenFile();
	}
}