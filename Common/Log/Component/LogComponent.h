#pragma once
#include<unordered_map>
#include"spdlog/spdlog.h"
#include"Component/Component.h"
#define LOG_FILE_MAX_SUM 10
#define LOG_FILE_MAX_SIZE 1024 * 1024 * 100

namespace Sentry
{
    class LogComponent final : public Component, public ISecondUpdate
	{
	 public:
		LogComponent() = default;
	 public:
		void SaveAllLog();
		void SaveLog(spdlog::level::level_enum type, const std::string& log);
#ifdef __ALL_OUTPUT_LOG__
        void OutputLog(spdlog::level::level_enum type, const std::string& log);
#endif
		void SaveLog(const std::string & name, spdlog::level::level_enum type, const std::string& log);
	 protected:
		bool Awake() final;
		void OnDestroy() final;
		void OnSecondUpdate(int tick) final;
	 private:
		std::shared_ptr<spdlog::logger> GetLogger(const std::string & name);
	 private:
		int mLogSaveTime;
		time_t mLastTime;
		std::string mServerName;
		std::string mLogSavePath;
#ifdef __ALL_OUTPUT_LOG__
      std::shared_ptr<spdlog::logger> mAllLog;
#endif
		std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> mLoggers;
	};
}