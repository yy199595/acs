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
    class LogComponent final : public Component, public ISecondUpdate
	{
	 public:
		LogComponent() = default;
	 public:
		void SaveAllLog();
#ifdef __DEBUG__
		void Output(spdlog::level::level_enum type, const std::string& log);
#endif
		void SaveLog(spdlog::level::level_enum type, const std::string& log);
		void SaveLog(const std::string & name, spdlog::level::level_enum type, const std::string& log);
	 protected:
		bool Awake() final;
		void OnDestroy() final;
		void OnSecondUpdate(const int tick) final;
	 private:
		std::shared_ptr<spdlog::logger> GetLogger(const std::string & name);
	 private:
		int mLogSaveTime;
		time_t mLastTime;
		std::string mServerName;
		std::string mLogSavePath;
#ifdef __DEBUG__
		std::shared_ptr<spdlog::logger> mAllLog;
#endif
		std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> mLoggers;
	};
}