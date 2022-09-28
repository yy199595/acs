#pragma once
#include<sstream>
#include<string>
#include<string.h>
#include"Log/Debug.h"
#define args1 std::placeholders::_1
#define args2 std::placeholders::_2
#define args3 std::placeholders::_3

inline std::string FormatFileLine(const char * file, const int line)
{
	const size_t length = strlen(file);
	const char * fileName = nullptr;

	for (size_t index = length - 1; index >= 0; index--)
	{
#ifdef _WIN32
		if (file[index] == '\\')
#else
		if(file[index] == '/')
#endif
		{
			fileName = file + index + 1;
			break;
		}
	}
	char buffer[100] = {};
	fileName = fileName == nullptr ? file : fileName;
#ifdef _MSC_VER
	size_t size = sprintf_s(buffer, "%s:%d  ", fileName, line);
#else
	size_t size = sprintf(buffer, "%s:%d  ", fileName, line);
#endif // _MSC_VER
	return std::string(buffer, size);
}


#define GK_LOG(type, f, content)						\
{                                     \
	std::stringstream ss;                \
	ss << f << content;\
    Debug::Log(type, ss.str());		\
}													\


#define LOG_INFO(content) \
{                             \
		std::string f = FormatFileLine(__FILE__, __LINE__); \
       	GK_LOG(spdlog::level::level_enum::info, f, content)        \
}

#define LOG_DEBUG(content) \
{                             \
		std::string f = FormatFileLine(__FILE__, __LINE__); \
       	GK_LOG(spdlog::level::level_enum::debug, f, content)        \
}

#define LOG_WARN(content) \
{                             \
       	std::string f = FormatFileLine(__FILE__, __LINE__); \
       	GK_LOG(spdlog::level::level_enum::warn, f, content)        \
}

#define LOG_ERROR(content) \
{                                      \
       	std::string f = FormatFileLine(__FILE__, __LINE__); \
       	GK_LOG(spdlog::level::level_enum::err, f, content)          \
}                              \

#define LOG_FATAL(content) \
{                                      \
       	std::string f = FormatFileLine(__FILE__, __LINE__); \
       	GK_LOG(spdlog::level::level_enum::critical, f, content)          \
}                              \


#define LOG_CHECK_FATAL(obj)			\
{												\
	if(!(obj))									\
	{											\
		LOG_FATAL(#obj);		                \
	}											\
}

#define LOG_CHECK_RET(obj) \
{                                \
    if(!(obj)) { LOG_ERROR(#obj); return; }    \
}

#define LOG_CHECK_RET_FALSE(obj) \
{                                \
    if(!(obj)) { LOG_ERROR(#obj); return false; }    \
}

#define LOG_CHECK_RET_ZERO(obj){ \
    if(!(obj)) { LOG_ERROR(#obj); return 0; }    \
}

#define LOG_CHECK_RET_NULL(obj){ \
    if(!(obj)) { LOG_ERROR(#obj); return nullptr; }    \
}

#define LOG_ERROR_RETURN_CODE(obj, code) \
    if(!(obj)) { LOG_ERROR(#obj); return code; }    \

#define LOG_RPC_CHECK_ARGS(obj) \
           if(!(obj)) { LOG_ERROR(#obj); return XCode::CallArgsError; }    \

#define IF_THROW_ERROR(obj){ \
    if(!(obj))                \
    {              \
        throw std::logic_error(#obj);\
        }    \
}

#define TryInvoke(obj, content){ \
        if(obj)                  \
        {                        \
            content;             \
        }\
}

#define CONSOLE_LOG_ERROR(content){ \
	std::string f = FormatFileLine(__FILE__, __LINE__); \
	std::stringstream ss;                \
	ss << f << content;                \
	Debug::Console(Debug::Level::err, ss.str());\
}

#define CONSOLE_LOG_FATAL(content){ \
	std::string f = FormatFileLine(__FILE__, __LINE__); \
	std::stringstream ss;                \
	ss << f << content;                \
	Debug::Console(Debug::Level::critical, ss.str());\
}

#define CONSOLE_LOG_INFO(content){ \
	std::string f = FormatFileLine(__FILE__, __LINE__); \
	std::stringstream ss;                \
	ss << f << content;                \
	Debug::Console(Debug::Level::info, ss.str());\
}

#define CONSOLE_LOG_DEBUG(content){ \
	std::string f = FormatFileLine(__FILE__, __LINE__); \
	std::stringstream ss;                \
	ss << f << content;                \
	Debug::Console(Debug::Level::debug, ss.str());\
}
