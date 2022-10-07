#pragma once
#include<unordered_map>
#include"spdlog/spdlog.h"
#include"Time/TimeHelper.h"
#include"Component/Component.h"
#define LOG_FILE_MAX_SUM 10
#define LOG_FILE_MAX_SIZE 1024 * 1024 * 100
using namespace Helper;
namespace Sentry
{
	class LoggerComponent final : public Component, public IZeroRefresh
	{
	 public:
		LoggerComponent() = default;
		~LoggerComponent() final = default;
	 public:
		void SaveAllLog();
		void AddLog(spdlog::level::level_enum type, const std::string& log);
	 protected:
		void Awake() final;
		void OnDestory() final;
		void OnZeroRefresh() final;
	public:
		void CreateLogFile();
	 private:
		int mLogSaveTime;
		std::string mServerName;
		std::string mLogSavePath;
#ifdef __DEBUG__
		std::shared_ptr<spdlog::logger> mAllLog;
#else
		std::shared_ptr<spdlog::logger> mInfoLog;
		std::shared_ptr<spdlog::logger> mDebugLog;
		std::shared_ptr<spdlog::logger> mErrorLog;
		std::shared_ptr<spdlog::logger> mFatalLog;
		std::shared_ptr<spdlog::logger> mWarningLog;
#endif
	};
}