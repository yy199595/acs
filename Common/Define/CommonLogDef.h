#pragma once
#include<sstream>
#include<string>
#include"CommonTypeDef.h"
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



#define GK_LOG(type, f, ...)						\
{                                 					\
    f.append(fmt::format(__VA_ARGS__));				\
    App::Get()->GetLogger()->AddLog(type, f);		\
}													\

#define LOG_INFO(...) \
{                             \
		std::string f = FormatFileLine(__FILE__, __LINE__); \
       	GK_LOG(spdlog::level::level_enum::info, f, __VA_ARGS__)        \
}

#define LOG_DEBUG(...) \
{                             \
		std::string f = FormatFileLine(__FILE__, __LINE__); \
       	GK_LOG(spdlog::level::level_enum::debug, f, __VA_ARGS__)        \
}

#define LOG_WARN(...) \
{                             \
       	std::string f = FormatFileLine(__FILE__, __LINE__); \
       	GK_LOG(spdlog::level::level_enum::warn, f, __VA_ARGS__)        \
}

#define LOG_ERROR(...) \
{                                      \
       	std::string f = FormatFileLine(__FILE__, __LINE__); \
       	GK_LOG(spdlog::level::level_enum::err, f, __VA_ARGS__)          \
}                              \

#define LOG_FATAL(...) \
{                                      \
       	std::string f = FormatFileLine(__FILE__, __LINE__); \
       	GK_LOG(spdlog::level::level_enum::critical, f, __VA_ARGS__)          \
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

#define LOGIC_THROW_ERROR(obj){ \
    if(!(obj))                \
    {              \
        LOG_ERROR(#obj);      \
        throw std::logic_error(#obj);\
        }    \
}

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
