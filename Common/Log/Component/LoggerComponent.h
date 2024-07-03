//
// Created by yy on 2023/8/12.
//

#ifndef APP_LOGGERCOMPONENT_H
#define APP_LOGGERCOMPONENT_H

#include<mutex>
#include"Core/Map/HashMap.h"
#include"Log/Common/Logger.h"
#include"Core/Thread/AsioThread.h"
#include"Entity/Component/Component.h"
namespace joke
{
#ifndef __DEBUG__
	struct LangText
	{
		std::string time;
		std::string file;
		std::string info;
		std::string title;
		std::string level;
		std::string debug;
		std::string error;
		std::string fatal;
		std::string trace;
		std::string warning;
		std::string content;
	};
#endif

	class LoggerComponent : public Component,
			public ILuaRegister, public IServerRecord, public IDestroy
	{
	public:
		LoggerComponent();
		~LoggerComponent();
	public:
		void Flush();
		void DropAllLog();
		void Flush(const std::string & name);
        void SetLogLevel(custom::LogLevel level);
		bool StartLogger(const std::string & name);
		custom::Logger * GetLogger(const std::string& name);
		void PushLog(std::unique_ptr<custom::LogInfo> logInfo);
		void PushLog(const std::string & name, std::unique_ptr<custom::LogInfo> logInfo);
	private:
		bool Awake() final;
		bool LateAwake() final;
		void OnDestroy() final;
		void OnRecord(json::w::Document &document) final;
		void OnLuaRegister(Lua::ModuleClass &luaRegister) final;
	private:
		std::mutex mMutex;
#ifndef __DEBUG__
		LangText mLangText;
#endif
		std::string mLogName;
		std::string mLogPath;
		custom::FileConfig mConf;
		custom::LogLevel mLogLevel;
		custom::Logger * mMainLogger;
		class ThreadComponent * mThread;
		class GroupNotifyComponent * mNotify;
		std::unordered_map<std::string, std::unique_ptr<custom::Logger>> mLoggers;
	};
}


#endif //APP_LOGGERCOMPONENT_H
