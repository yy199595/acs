#pragma once
#include <unordered_map>
#include<spdlog/spdlog.h>
#include<Define/CommonTypeDef.h>
#include<spdlog/sinks/basic_file_sink.h>
#include<spdlog/sinks/stdout_color_sinks.h>
namespace Sentry
{

    typedef spdlog::level::level_enum ELogType;
	class LogHelper{
	public:
		static bool Init(const std::string path, const std::string name);
	public:
		static void AddLog(ELogType type, const std::string & log);
	private:
        static std::shared_ptr<spdlog::logger> mErrorLog;
        static std::shared_ptr<spdlog::logger> mFatalLog;
        static std::shared_ptr<spdlog::logger> mWarningLog;
        static std::shared_ptr<spdlog::logger> mRecordLog;
	};
}