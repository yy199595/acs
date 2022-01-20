#pragma once
#include"Component.h"
#include<unordered_map>
#include<spdlog/spdlog.h>
#include"Util/TimeHelper.h"
#include"Other/StringFmt.h"
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
        void AddLog(ELogType type, Args &&... args);
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
        static thread_local std::string mLogBuffer;
		std::shared_ptr<spdlog::logger> mInfoLog;
		std::shared_ptr<spdlog::logger> mDebugLog;
		std::shared_ptr<spdlog::logger> mErrorLog;
		std::shared_ptr<spdlog::logger> mFatalLog;
		std::shared_ptr<spdlog::logger> mRecordLog;
		std::shared_ptr<spdlog::logger> mWarningLog;
	};

    template<typename ... Args>
    void LoggerComponent::AddLog(ELogType type,  Args &&...args)
    {
        switch (type)
        {
            case ELogType::info:
            {
                this->mInfoLog->info(Fmt::Fotmat(mLogBuffer, std::forward<Args>(args)...));
#ifdef __DEBUG__
                this->AddInfoLog(Fmt::Fotmat(mLogBuffer, std::forward<Args>(args)...));
#endif
            }
                break;
            case ELogType::debug:
            {
                this->mDebugLog->debug(Fmt::Fotmat(mLogBuffer, std::forward<Args>(args)...));
#ifdef __DEBUG__
                this->AddDebugLog(Fmt::Fotmat(mLogBuffer, std::forward<Args>(args)...));
#endif
            }
                break;
            case ELogType::warn:
            {
                this->mWarningLog->warn(Fmt::Fotmat(mLogBuffer, std::forward<Args>(args)...));
#ifdef __DEBUG__
                this->AddWarningLog(Fmt::Fotmat(mLogBuffer, std::forward<Args>(args)...));
#endif
            }
                break;
            case ELogType::err:
            {
                this->mErrorLog->error(Fmt::Fotmat(mLogBuffer,std::forward<Args>(args) ...));
#ifdef __DEBUG__
                this->AddErrorLog(Fmt::Fotmat(mLogBuffer,std::forward<Args>(args)...));
#endif
                break;
            }
            case ELogType::critical:
            {
                this->mFatalLog->critical(Fmt::Fotmat(mLogBuffer,std::forward<Args>(args) ...));
#ifdef __DEBUG__
                this->AddFatalLog(Fmt::Fotmat(mLogBuffer,std::forward<Args>(args)...));
#endif
            }
                break;
        }
    }
}