//
// Created by yy on 2023/8/12.
//

#ifndef APP_LEVEL_H
#define APP_LEVEL_H
#include<string>
#ifdef __SHARE_PTR_COUNTER__
#include "Core/Memory/MemoryObject.h"
#endif
namespace custom
{
	enum class LogLevel
	{
		None = 0,
		Debug = LOG_LEVEL_DEBUG,
		Info = LOG_LEVEL_INFO,
		Warn = LOG_LEVEL_WARN,
		Error = LOG_LEVEL_ERROR,
		Fatal = LOG_LEVEL_FATAL,
		OFF = LOG_LEVEL_OFF,
		All = 128,
	};

	struct LogInfo
#ifdef __SHARE_PTR_COUNTER__
	: public memory::Object<LogInfo>
#endif
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
