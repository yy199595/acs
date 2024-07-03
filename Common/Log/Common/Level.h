//
// Created by yy on 2023/8/12.
//

#ifndef APP_LEVEL_H
#define APP_LEVEL_H
#include<string>
#ifdef __ENABLE_SPD_LOG__
#include<spdlog/logger.h>
#endif

namespace custom
{
	enum class LogLevel
	{
		None = 0,
#ifdef __ENABLE_SPD_LOG__
		Debug 	= spdlog::level::debug,
		Info  	= spdlog::level::info,
		Warn 	= spdlog::level::warn,
		Error 	= spdlog::level::err,
		Fatal	= spdlog::level::critical
#else
		Debug = LOG_LEVEL_DEBUG,
		Info = LOG_LEVEL_INFO,
		Warn = LOG_LEVEL_WARN,
		Error = LOG_LEVEL_ERROR,
		Fatal = LOG_LEVEL_FATAL,
		OFF = LOG_LEVEL_OFF,
#endif
	};

	struct LogInfo
	{
	public:
		LogLevel Level;
		std::string File;
		bool Flush = false;
		std::string Stack;
		std::string Content;
	public:
		inline void Clear()
		{
			this->File.clear();
			this->Stack.clear();
			this->Content.clear();
			this->Level = LogLevel::None;
		}
	};
}

#endif //APP_LEVEL_H
