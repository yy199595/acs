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
	fileName = fileName == nullptr ? file : fileName;
	char buffer[100] = {};
#ifdef _MSC_VER
	size_t size = sprintf_s(buffer, "%s:%d", fileName, line);
#else
	size_t size = sprintf(buffer, "%s:%d", fileName, line);
#endif // _MSC_VER
	return {buffer, size};
}

#define GK_LOG(type, msg)						\
{												\
	std::stringstream ss; ss << msg;			\
	App::Get().GetLogger()->AddLog(type, ss);	\
}												\

#define GK_CommonLog(msg)		GK_LOG(GameKeeper::ELogType::debug, msg)
#define GKCommonError(msg)	GK_LOG(GameKeeper::ELogType::err, msg)
#define GKCommonWarning(msg) GK_LOG(GameKeeper::ELogType::warn, msg)
#define GKCommonFatal(msg)	GK_LOG(GameKeeper::ELogType::critical, msg)
#define GKCommonInfo(msg)	GK_LOG(GameKeeper::ELogType::info, msg)

#define LOG_DEBUG(msg)		GK_LOG(GameKeeper::ELogType::debug, FormatFileLine(__FILE__, __LINE__) << "  " << msg)
#define LOG_WARN(msg)	GK_LOG(GameKeeper::ELogType::warn, FormatFileLine(__FILE__, __LINE__) << "  " << msg)
#define LOG_ERROR(msg)	GK_LOG(GameKeeper::ELogType::err, FormatFileLine(__FILE__, __LINE__) << "  " << msg)
#define LOG_FATAL(msg)	GK_LOG(GameKeeper::ELogType::critical, FormatFileLine(__FILE__, __LINE__) << "  " << msg)
#define LOG_INFO(msg)		GK_LOG(GameKeeper::ELogType::info, FormatFileLine(__FILE__, __LINE__) << "  " << msg)


#define LOG_CHECK_ERROR(obj, msg, val)			\
{												\
	if(!(obj))									\
	{											\
		LOG_ERROR(#obj<< "  "  << msg);		\
		val;									\
	}											\
}

#define LOG_CHECK_FATAL(obj)			\
{												\
	if(!(obj))									\
	{											\
		LOG_FATAL(#obj);		                \
	}											\
}

#define LOG_CHECK_WARN(obj, msg)			\
{												\
	if(!(obj))									\
	{											\
		LOG_WARN(#obj << "  " << msg);	\
		return;									\
	}											\
}

#define LOG_CHECK_ERROR_RET(obj, msg)			LOG_CHECK_ERROR(obj, msg, return)
#define LOG_CHECK_ERROR_RET_VAL(obj, msg, val)	LOG_CHECK_ERROR(obj, msg, return val)
#define LOG_CHECK_ERROR_RET_NULL(obj, msg)		LOG_CHECK_ERROR(obj, msg, return nullptr)

#define LOG_CHECK_RET_ZERO(obj)	LOG_CHECK_ERROR_RET_VAL(obj, "", 0)
#define LOG_CHECK_RET(obj)		LOG_CHECK_ERROR_RET(obj, "")
#define LOG_CHECK_RET_FALSE(obj)	LOG_CHECK_ERROR_RET_VAL(obj, "", false)


#define GKAssertLog(obj, msg){ bool bCode = !(obj);if (bCode) { LOG_ERROR(#obj<< "  "  << msg); } }
#define GKAssertWarning(obj, msg){bool bCode = !(obj);	if (bCode) { LOG_WARN(#obj<< "  "  << msg); } }

#define SOEASY_DEBUG