#pragma once
#include"Component.h"
#include<unordered_map>
#include<spdlog/spdlog.h>
#include"Util/TimeHelper.h"
#define LOG_FILE_MAX_SUM 10
#define LOG_FILE_MAX_SIZE 1024 * 1024 * 100
using namespace Helper;
namespace Sentry
{
	typedef spdlog::level::level_enum ELogType;
	class LoggerComponent : public Component, public IZeroRefresh
	{
	public:
		LoggerComponent() = default;
		~LoggerComponent() final = default;
	public:
        template<typename ... Args>
        void AddLog(ELogType type, const std::string & line, const std::string & format, Args &&... args);
	protected:	
		bool Awake() final;
        bool LateAwake() final;
		void OnDestory() final;
		void OnZeroRefresh() final;
	private:
		void CreateLogFile();
		void AddInfoLog(const std::string & log);
		void AddErrorLog(const std::string & log);
		void AddDebugLog(const std::string & log);
		void AddFatalLog(const std::string & log);
		void AddWarningLog(const std::string & log);
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
		std::shared_ptr<spdlog::logger> mRecordLog;
		std::shared_ptr<spdlog::logger> mWarningLog;
#endif
	};

    template<typename ... Args>
    void LoggerComponent::AddLog(ELogType type, const std::string & line, const std::string & format,  Args &&...args)
    {
        switch (type)
        {
            case ELogType::info:
            {
				std::string str;
				fmt::format(format, std::forward<Args>(args)...);
#ifdef __DEBUG__
				this->mAllLog->info(format, std::forward<Args>(args)...);
				this->AddInfoLog(fmt::format(format, std::forward<Args>(args)...));
#else
				this->mInfoLog->info(format, std::forward<Args>(args)...);
#endif
            }
                break;
            case ELogType::debug:
            {
#ifdef __DEBUG__
				this->mAllLog->debug(format, std::forward<Args>(args)...);
				this->AddDebugLog(fmt::format(format, std::forward<Args>(args)...));
#else
				this->mDebugLog->debug(format, std::forward<Args>(args)...);
#endif
            }
                break;
            case ELogType::warn:
            {
#ifdef __DEBUG__
				this->mAllLog->warn(format, std::forward<Args>(args)...);
				this->AddWarningLog(fmt::format(format, std::forward<Args>(args)...));
#else
				this->mWarningLog->warn(format, std::forward<Args>(args)...);
#endif
            }
                break;
            case ELogType::err:
            {
#ifdef __DEBUG__
				this->mAllLog->error(format, std::forward<Args>(args)...);
				this->AddErrorLog(fmt::format(format, std::forward<Args>(args)...));
#else
				this->mErrorLog->error(format,std::forward<Args>(args) ...);
#endif
                break;
            }
            case ELogType::critical:
            {
#ifdef __DEBUG__
				this->mAllLog->critical(format, std::forward<Args>(args)...);
				this->AddFatalLog(fmt::format(format, std::forward<Args>(args)...));
#else
				this->mFatalLog->critical(format,std::forward<Args>(args) ...);
#endif
            }
                break;
        }
    }
}