//
// Created by yy on 2023/8/12.
//

#include "LoggerComponent.h"
#include "Entity/Actor/App.h"
#include "Core/System/System.h"
#include "Lua/Engine/ModuleClass.h"
#include "Log/Lua/LuaLogger.h"
#include "Util/Time/TimeHelper.h"
#include "Log/Output/FileOutput.h"
#include "Log/Output/ShowOutput.h"
#include "Log/Output/MongoOutput.h"
#include "Config/Base/LangConfig.h"
#ifdef __ENABLE_DING_DING_PUSH
#include "Http/Common/HttpRequest.h"
#include "Http/Common/HttpResponse.h"
#include "Http/Component/HttpComponent.h"
#endif
#include "Http/Component/GroupNotifyComponent.h"
#include "Server/Component/ThreadComponent.h"

namespace joke
{
	LoggerComponent::LoggerComponent()
	{
		this->mLogPath = "./log";
		this->mNotify = nullptr;
		this->mMainLogger = nullptr;
		this->mLogLevel = custom::LogLevel::Debug;
	}

	LoggerComponent::~LoggerComponent() noexcept
	{
		this->mLoggers.clear();
	}

	bool LoggerComponent::Awake()
	{
		std::string name;
		System::GetEnv("name", name);
		this->mLogLevel = custom::LogLevel::Debug;
		std::unique_ptr<json::r::Value> jsonObject;
		this->mThread = this->GetComponent<ThreadComponent>();
		if (this->mApp->Config().Get("log", jsonObject))
		{
			int level = 0;
			if (jsonObject->Get("level", level))
			{
				this->mLogLevel = (custom::LogLevel)level;
			}
			jsonObject->Get("path", this->mLogPath);
			jsonObject->Get("max_size", this->mConf.MaxSize);
			jsonObject->Get("max_line", this->mConf.MaxLine);
		}
		int id = this->mApp->GetSrvId();
		this->mLogName = fmt::format("{}-{}", name, id);
		this->mMainLogger = this->GetLogger(this->mLogName);
		if (!this->mMainLogger->Start())
		{
			return false;
		}
		this->mMainLogger->SetLevel(this->mLogLevel);
#ifndef __DEBUG__
		LOG_CHECK_RET_FALSE(LangConfig::Inst()->Get("time", this->mLangText.time))
		LOG_CHECK_RET_FALSE(LangConfig::Inst()->Get("file", this->mLangText.file))
		LOG_CHECK_RET_FALSE(LangConfig::Inst()->Get("info", this->mLangText.info))
		LOG_CHECK_RET_FALSE(LangConfig::Inst()->Get("fatal", this->mLangText.fatal))
		LOG_CHECK_RET_FALSE(LangConfig::Inst()->Get("debug", this->mLangText.debug))
		LOG_CHECK_RET_FALSE(LangConfig::Inst()->Get("level", this->mLangText.level))
		LOG_CHECK_RET_FALSE(LangConfig::Inst()->Get("error", this->mLangText.error))
		LOG_CHECK_RET_FALSE(LangConfig::Inst()->Get("warn", this->mLangText.warning))
		LOG_CHECK_RET_FALSE(LangConfig::Inst()->Get("content", this->mLangText.content))
		LOG_CHECK_RET_FALSE(LangConfig::Inst()->Get("server_log_notify", this->mLangText.title))
#endif
		return true;
	}

	bool LoggerComponent::LateAwake()
	{
		this->mNotify = this->GetComponent<GroupNotifyComponent>();
		return true;
	}

	void LoggerComponent::OnLuaRegister(Lua::ModuleClass &luaRegister)
	{
		luaRegister.AddFunction("Output", Lua::Log::Output);
		luaRegister.AddFunction("Show", Lua::Console::Show).End("util.logger");
	}

	void LoggerComponent::OnRecord(json::w::Document &document)
	{
		std::unique_ptr<json::w::Value> data = document.AddObject("log");
		{
			data->Add("level", (int)this->mLogLevel);
			data->Add("count", this->mLoggers.size());
		}
	}

	void LoggerComponent::OnDestroy()
	{
		this->DropAllLog();
	}

	void LoggerComponent::Flush()
	{
		this->mMainLogger->Flush();
	}

	void LoggerComponent::SetLogLevel(custom::LogLevel level)
	{
		this->mLogLevel = level;
	}

	void LoggerComponent::DropAllLog()
	{
		auto iter = this->mLoggers.begin();
		for (; iter != this->mLoggers.end(); iter++)
		{
			iter->second->Close();
		}
	}

	void LoggerComponent::Flush(const std::string &name)
	{
		custom::Logger *logger = nullptr;
		std::lock_guard<std::mutex> lock(this->mMutex);
		auto iter = this->mLoggers.find(name);
		if(iter != this->mLoggers.end())
		{
			iter->second->Flush();
		}
	}

	void LoggerComponent::PushLog(std::unique_ptr<custom::LogInfo> log)
	{
		assert(this->mApp->IsMainThread());
		if (log->Level >= this->mLogLevel)
		{
#ifndef __DEBUG__
			if(this->mNotify != nullptr && log->Level >= custom::LogLevel::Error)
			{
				std::string value;
				notify::Markdown markdownInfo;
				markdownInfo.title = this->mLangText.title;
				switch (log->Level)
				{
					case custom::LogLevel::Debug:
						value = this->mLangText.debug;
						break;
					case custom::LogLevel::Info:
						value = this->mLangText.info;
						break;
					case custom::LogLevel::Warn:
						value = this->mLangText.warning;
						break;
					case custom::LogLevel::Error:
						value = this->mLangText.error;
						break;
					case custom::LogLevel::Fatal:
						value = this->mLangText.fatal;
						break;
				}
				markdownInfo.data.emplace_back(this->mLangText.level, value);
				markdownInfo.data.emplace_back(this->mLangText.time, help::Time::GetDateString());
				markdownInfo.data.emplace_back(this->mLangText.file, log->File);
				if (!log->Stack.empty())
				{
					markdownInfo.data.emplace_back(this->mLangText.trace, log->Stack);
				}
				markdownInfo.data.emplace_back(this->mLangText.content, log->Content);
				this->mNotify->SendToWeChat(markdownInfo);
			}
#endif
			this->mMainLogger->Push(std::move(log));
		}
	}

	void LoggerComponent::PushLog(const std::string &name, std::unique_ptr<custom::LogInfo> logInfo)
	{
		custom::Logger *logger = nullptr;
		std::lock_guard<std::mutex> lock(this->mMutex);
		auto iter = this->mLoggers.find(name);
		if(iter != this->mLoggers.end())
		{
			iter->second->Push(std::move(logInfo));
		}
	}

	bool LoggerComponent::StartLogger(const std::string &name)
	{
		return this->GetLogger(name)->Start();
	}

	custom::Logger *LoggerComponent::GetLogger(const std::string &name)
	{
		std::lock_guard<std::mutex> lock(this->mMutex);
		auto iter = this->mLoggers.find(name);
		if(iter != this->mLoggers.end())
		{
			return iter->second.get();
		}
		custom::Logger * result = nullptr;
		custom::FileConfig fileConfig = this->mConf;
		std::unique_ptr<custom::Logger> logger = std::make_unique<custom::Logger>(name, this->mThread->GetContext());
		{
			result = logger.get();
			fileConfig.Name = name;
			fileConfig.Root = this->mLogPath;
			fileConfig.Server = this->mApp->Name();
			logger->AddOutput<custom::FileOutput>(fileConfig);
		}
		this->mLoggers.emplace(name, std::move(logger));
		return result;
	}
}