#pragma once
#include<unordered_map>
#include"spdlog/spdlog.h"
#include"Core/Component/Component.h"
#define LOG_FILE_MAX_SUM 10
#define LOG_FILE_MAX_SIZE 1024 * 1024 * 100

namespace Tendo
{
    class LogComponent final : public Component, public ISecondUpdate
	{
	 public:
		LogComponent() = default;
	 public:
		void SaveAllLog();
		void SaveLog(spdlog::level::level_enum type, const std::string& log);
		void SaveLog(const std::string & name, spdlog::level::level_enum type, const std::string& log);
	 protected:
		bool Awake() final;
		void OnSecondUpdate(int tick) final;
	 private:
		std::shared_ptr<spdlog::logger> GetLogger(const std::string & name);
	 private:
		int mLogSaveTime;
		time_t mLastTime;
		std::string mServerName;
		std::string mLogSavePath;
		std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> mLoggers;
	};
}