#pragma once
#include"Component.h"
#include<unordered_map>
#include<spdlog/spdlog.h>
#define LOG_FILE_MAX_SUM 10
#define LOG_FILE_MAX_SIZE 1024 * 1024 * 100

namespace GameKeeper
{
	typedef spdlog::level::level_enum ELogType;
	class LoggerComponent : public Component, public IZeroRefresh
	{
	public:
		LoggerComponent() = default;
		~LoggerComponent() final = default;
	public:
		void AddLog(ELogType type, const std::stringstream & stream);
	protected:	
		bool Awake() final;
		void OnDestory() final;
		void OnZeroRefresh() final;
	private:
		void CreateLogFile();
		void AddInforLog(const std::stringstream & stream);
		void AddErrorLog(const std::stringstream & stream);
		void AddDebugLog(const std::stringstream & stream);
		void AddFatalLog(const std::stringstream & stream);
		void AddWarningLog(const std::stringstream & stream);
	private:
		int mLogSaveTime;
		std::string mServerName;
		std::string mLogSavePath;
		std::stringstream mLogStream;	
		std::shared_ptr<spdlog::logger> mInfoLog;
		std::shared_ptr<spdlog::logger> mDebugLog;
		std::shared_ptr<spdlog::logger> mErrorLog;
		std::shared_ptr<spdlog::logger> mFatalLog;
		std::shared_ptr<spdlog::logger> mRecordLog;
		std::shared_ptr<spdlog::logger> mWarningLog;
	};
}