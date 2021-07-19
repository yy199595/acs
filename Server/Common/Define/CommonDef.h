#pragma once
#include<sstream>
#include<Util/ProtocHelper.h>
#include<Global/LogHelper.h>
#define DeletePtr(obj) if(obj != nullptr) {delete obj; obj = nullptr; }
#define args1 std::placeholders::_1
#define args2 std::placeholders::_2
#define args3 std::placeholders::_3
#define args4 std::placeholders::_4
#define args5 std::placeholders::_5
#define ASIO_TCP_SEND_MAX_COUNT 1024 * 1024

#define if_false(code, content) if (!code) { content; }
inline std::string FromatFileLine(const char * file, const int line)
{
	const size_t lenght = strlen(file);
	const char * fileName = nullptr;

	for (size_t index = lenght - 1; index >= 0; index--)
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
#endif // _MSG
	return std::string(buffer, size);	
}

#define SAYNO_LOG(type, msg) {std::stringstream ss; ss << msg; Sentry::LogHelper::AddLog(type, ss.str());}

#define TypeLimit(base,child) typename std::enable_if<std::is_base_of<base, child>::value, T>::type

#define SayNoCommonLog(msg)		SAYNO_LOG(Sentry::ELogType::debug, msg)
#define SayNoCommonError(msg)	SAYNO_LOG(Sentry::ELogType::err, msg)
#define SayNoCommonWarning(msg) SAYNO_LOG(Sentry::ELogType::warn, msg)
#define SayNoCommonFatal(msg)	SAYNO_LOG(Sentry::ELogType::critical, msg)
#define SayNoCommonInfo(msg)	SAYNO_LOG(Sentry::ELogType::info, msg)

#define SayNoDebugLog(msg)		SAYNO_LOG(Sentry::ELogType::debug, FromatFileLine(__FILE__, __LINE__) << "  " << msg)
#define SayNoDebugWarning(msg)	SAYNO_LOG(Sentry::ELogType::warn, FromatFileLine(__FILE__, __LINE__) << "  " << msg)
#define SayNoDebugError(msg)	SAYNO_LOG(Sentry::ELogType::err, FromatFileLine(__FILE__, __LINE__) << "  " << msg)
#define SayNoDebugFatal(msg)	SAYNO_LOG(Sentry::ELogType::critical, FromatFileLine(__FILE__, __LINE__) << "  " << msg)
#define SayNoDebugInfo(msg)		SAYNO_LOG(Sentry::ELogType::info, FromatFileLine(__FILE__, __LINE__) << "  " << msg)


#define SayNoAssertError(obj, msg, val) { bool bCode = !(obj); if(bCode) { SayNoDebugError(#obj<< "  "  << msg); val; } }
#define SayNoAssertFatal(obj, msg, val) { bool bCode = !(obj); if(bCode) { SayNoDebugFatal(#obj<< "  "  << msg); val; } }

#define SayNoAssertRetWarning(obj, msg)	{bool bResult = !(obj); if(bResult) { SayNoDebugWarning(#obj << "  " << msg);return;} }

#define SayNoAssertRet(obj, msg)			SayNoAssertError(obj, msg, return)
#define SayNoAssertRetVal(obj, msg, val)	SayNoAssertError(obj, msg, return val)
#define SayNoAssertRetNull(obj, msg)		SayNoAssertError(obj, msg, return nullptr)
#define SayNoAssertRetFail(obj, msg)		SayNoAssertError(obj, msg, return XCode::Failure)

#define SayNoAssertRetCode(obj,code) SayNoAssertRetVal(obj, "", code)
#define SayNoAssertRetZero_F(obj)	SayNoAssertRetVal(obj, "", 0)
#define SayNoAssertRet_F(obj)		SayNoAssertRet(obj, "")
#define SayNoAssertRetFail_F(obj)	SayNoAssertRetFail(obj, "")
#define SayNoAssertRetNull_F(obj)	SayNoAssertRetNull(obj, "")
#define SayNoAssertRetFalse_F(obj)	SayNoAssertRetVal(obj, "", false)
#define SayNoAssertBreakFatal_F(obj) { bool bCode = !(obj); if(bCode) { SayNoDebugFatal(#obj)  cin.get(); exit(-1);}  }

#define SayNoAssertLog(obj, msg){ bool bCode = !(obj);if (bCode) { SayNoDebugError(#obj<< "  "  << msg); } }
#define SayNoAssertWarning(obj, msg){bool bCode = !(obj);	if (bCode) { SayNoDebugWarning(#obj<< "  "  << msg); } }

#define SayNoDebugLogProtocBuf(msg) { std::string str; ProtocHelper::GetJsonString(msg, str); SayNoDebugLog(str);}

#define BIND_ACTION_0(func, obj) std::bind(&func, obj)
#define BIND_ACTION_1(func, obj) std::bind(&func, obj, args1)
#define BIND_ACTION_2(func, obj) std::bind(&func, obj, args1, args2)
#define BIND_ACTION_3(func, obj) std::bind(&func, obj, args1, args2, args3)
#define BIND_ACTION_4(func, obj) std::bind(&func, obj, args1, args2, args3, args4)

#define BIND_THIS_ACTION_0(func) std::bind(&func, this)
#define BIND_THIS_ACTION_1(func) std::bind(&func, this, args1)
#define BIND_THIS_ACTION_2(func) std::bind(&func, this, args1, args2)
#define BIND_THIS_ACTION_3(func) std::bind(&func, this, args1, args2, args3)
#define BIND_THIS_ACTION_4(func) std::bind(&func, this, args1, args2, args3, args4)

#define SOEASY_DEBUG