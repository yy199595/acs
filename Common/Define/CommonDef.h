#pragma once
#include<sstream>
#include<Util/ProtocHelper.h>
#include<Global/LogHelper.h>
#define args1 std::placeholders::_1
#define args2 std::placeholders::_2
#define args3 std::placeholders::_3
#define args4 std::placeholders::_4
#define args5 std::placeholders::_5
#define TCP_SEND_MAX_COUNT 1024 * 1024

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

#define GK_LOG(type, msg) {std::stringstream ss; ss << msg; GameKeeper::LogHelper::AddLog(type, ss.str());}

#define TypeLimit(base,child) typename std::enable_if<std::is_base_of<base, child>::value, T>::type

#define GK_CommonLog(msg)		GK_LOG(GameKeeper::ELogType::debug, msg)
#define GKCommonError(msg)	GK_LOG(GameKeeper::ELogType::err, msg)
#define GKCommonWarning(msg) GK_LOG(GameKeeper::ELogType::warn, msg)
#define GKCommonFatal(msg)	GK_LOG(GameKeeper::ELogType::critical, msg)
#define GKCommonInfo(msg)	GK_LOG(GameKeeper::ELogType::info, msg)

#define GKDebugLog(msg)		GK_LOG(GameKeeper::ELogType::debug, FromatFileLine(__FILE__, __LINE__) << "  " << msg)
#define GKDebugWarning(msg)	GK_LOG(GameKeeper::ELogType::warn, FromatFileLine(__FILE__, __LINE__) << "  " << msg)
#define GKDebugError(msg)	GK_LOG(GameKeeper::ELogType::err, FromatFileLine(__FILE__, __LINE__) << "  " << msg)
#define GKDebugFatal(msg)	GK_LOG(GameKeeper::ELogType::critical, FromatFileLine(__FILE__, __LINE__) << "  " << msg)
#define GKDebugInfo(msg)		GK_LOG(GameKeeper::ELogType::info, FromatFileLine(__FILE__, __LINE__) << "  " << msg)


#define GKAssertError(obj, msg, val) { bool bCode = !(obj); if(bCode) { GKDebugError(#obj<< "  "  << msg); val; } }
#define GKAssertFatal(obj, msg, val) { bool bCode = !(obj); if(bCode) { GKDebugFatal(#obj<< "  "  << msg); val; } }

#define GKAssertRetWarning(obj, msg)	{bool bResult = !(obj); if(bResult) { GKDebugWarning(#obj << "  " << msg);return;} }

#define GKAssertRet(obj, msg)			GKAssertError(obj, msg, return)
#define GKAssertRetVal(obj, msg, val)	GKAssertError(obj, msg, return val)
#define GKAssertRetNull(obj, msg)		GKAssertError(obj, msg, return nullptr)
#define GKAssertRetFail(obj, msg)		GKAssertError(obj, msg, return XCode::Failure)

#define GKAssertRetCode_F(obj) GKAssertRetVal(obj, "", XCode::Failure)
#define GKAssertRetCode(obj,code) GKAssertRetVal(obj, "", code)
#define GKAssertRetZero_F(obj)	GKAssertRetVal(obj, "", 0)
#define GKAssertRet_F(obj)		GKAssertRet(obj, "")
#define GKAssertRetFail_F(obj)	GKAssertRetFail(obj, "")
#define GKAssertRetNull_F(obj)	GKAssertRetNull(obj, "")
#define GKAssertRetFalse_F(obj)	GKAssertRetVal(obj, "", false)
#define GKAssertBreakFatal_F(obj) { bool bCode = !(obj); if(bCode) { GKDebugFatal(#obj)  cin.get(); exit(-1);}  }

#define GKAssertLog(obj, msg){ bool bCode = !(obj);if (bCode) { GKDebugError(#obj<< "  "  << msg); } }
#define GKAssertWarning(obj, msg){bool bCode = !(obj);	if (bCode) { GKDebugWarning(#obj<< "  "  << msg); } }

#define GKDebugLogProtocBuf(msg) { std::string str; ProtocHelper::GetJsonString(msg, str); GKDebugLog(str);}

#define SOEASY_DEBUG