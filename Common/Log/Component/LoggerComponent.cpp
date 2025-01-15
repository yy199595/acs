//
// Created by yy on 2023/8/12.
//

#include "LoggerComponent.h"
#include "Entity/Actor/App.h"
#include "Core/System/System.h"
#include "Lua/Engine/ModuleClass.h"
#include "Log/Lua/LuaLogger.h"
#include "Util/Tools/TimeHelper.h"
#include "Log/Output/FileOutput.h"
#ifdef __CONSOLE_LOG__
#include "Log/Output/ConsoleOutput.h"
#endif
#include "Log/Output/WeChatOutput.h"
#include "Log/Output/MongoOutput.h"

#include "Config/Base/LangConfig.h"
#ifdef __ENABLE_DING_DING_PUSH
#include "Http/Common/HttpRequest.h"
#include "Http/Common/HttpResponse.h"
#include "Http/Component/HttpComponent.h"
#endif
#include "Http/Component/NotifyComponent.h"
#include "Server/Component/ThreadComponent.h"
#include "Lua/Lib/Lib.h"
namespace acs
{
	LoggerComponent::LoggerComponent()
	{
		this->mThread = nullptr;
	}

	LoggerComponent::~LoggerComponent()
	{
		this->mLoggers.clear();
	}

	bool LoggerComponent::Awake()
	{
		std::string name;
		os::System::GetEnv("name", name);
		std::unique_ptr<json::r::Value> jsonArray;
		this->mThread = this->GetComponent<ThreadComponent>();

		std::unordered_map<std::string, custom::LogLevel> logLevelMap = {
				{ "debug", custom::LogLevel::Debug, },
				{ "info", custom::LogLevel::Info, },
				{ "warn", custom::LogLevel::Warn, },
				{ "error", custom::LogLevel::Error, },
				{ "fatal", custom::LogLevel::Fatal, },
		};

		if (this->mApp->Config().Get("log", jsonArray))
		{
			for(size_t  index = 0; index < jsonArray->MemberCount(); index++)
			{
				std::unique_ptr<json::r::Value> jsonObject;
				if (!jsonArray->Get(index, jsonObject))
				{
					return false;
				}
				std::string level;
				custom::LogConfig config;
				config.max_line = 1024 * 10;
				config.max_line = 1024 * 1024;
				LOG_CHECK_RET_FALSE(jsonObject->Get("level", level));
				LOG_CHECK_RET_FALSE(jsonObject->Get("name", config.name))
				auto iter = logLevelMap.find(level);
				if(iter == logLevelMap.end())
				{
					return false;
				}
				config.level = iter->second;
				jsonObject->Get("wx", config.wx);
				jsonObject->Get("open", config.open);
				jsonObject->Get("ding", config.ding);

				jsonObject->Get("pem", config.pem);
				jsonObject->Get("path", config.path);
				jsonObject->Get("mongo", config.mongo);
				jsonObject->Get("max_size", config.max_size);
				jsonObject->Get("max_line", config.max_line);

				jsonObject->Get("console", config.console);

				this->mConfig.emplace_back(config);
			}
		}

		for(const custom::LogConfig & config : this->mConfig)
		{
			if(config.open && !this->Create(config))
			{
				return false;
			}
		}
		return true;
	}

	void LoggerComponent::OnDestroy()
	{
		this->DropAllLog();
	}

	void LoggerComponent::Flush()
	{
		std::lock_guard<std::mutex> lock(this->mMutex);
		auto iter1 = this->mLoggers.begin();
		for (; iter1 != this->mLoggers.end(); iter1++)
		{
			iter1->second->Flush();
		}
		auto iter = this->mLevelLoggers.begin();
		for (; iter != this->mLevelLoggers.end(); iter++)
		{
			iter->second->Flush();
		}
	}

	void LoggerComponent::DropAllLog()
	{
		auto iter = this->mLoggers.begin();
		for (; iter != this->mLoggers.end(); iter++)
		{
			iter->second->Close();
		}
		auto iter1 = this->mLevelLoggers.begin();
		for (; iter1 != this->mLevelLoggers.end(); iter1++)
		{
			iter1->second->Close();
		}
	}

	void LoggerComponent::Flush(const std::string& name)
	{
		std::lock_guard<std::mutex> lock(this->mMutex);
		auto iter = this->mLoggers.find(name);
		if (iter != this->mLoggers.end())
		{
			iter->second->Flush();
		}
	}

	void LoggerComponent::PushLog(std::unique_ptr<custom::LogInfo> log)
	{
		custom::Logger* logger = this->GetLogger(log->Level);
		if (logger != nullptr)
		{
			logger->Push(std::move(log));
		}
	}

	void LoggerComponent::PushLog(const std::string &name, std::unique_ptr<custom::LogInfo> logInfo)
	{
		std::lock_guard<std::mutex> lock(this->mMutex);
		auto iter = this->mLoggers.find(name);
		if(iter != this->mLoggers.end())
		{
			iter->second->Push(std::move(logInfo));
		}
	}

	custom::Logger *LoggerComponent::GetLogger(const std::string &name)
	{
		std::lock_guard<std::mutex> lock(this->mMutex);
		auto iter = this->mLoggers.find(name);
		return iter != this->mLoggers.end() ? iter->second : nullptr;
	}

	custom::Logger* LoggerComponent::GetLogger(custom::LogLevel logLevel)
	{
		std::lock_guard<std::mutex> lock(this->mMutex);
		auto iter = this->mLevelLoggers.find(logLevel);
		return iter != this->mLevelLoggers.end() ? iter->second : nullptr;
	}

	bool LoggerComponent::Create(const custom::LogConfig & config)
	{
		custom::Logger * logger = new custom::Logger(config.name, this->mThread->GetContext());
		{
			if (!config.path.empty())
			{
				custom::FileConfig fileConfig;
				fileConfig.Name = config.name;
				fileConfig.MaxLine = config.max_line;
				fileConfig.MaxSize = config.max_size;
				const std::string & server = this->mApp->Name();
				fileConfig.Root = fmt::format("{}/{}",config.path, server) ;
				logger->AddOutput<custom::FileOutput>(fileConfig);
			}
			if (!config.wx.empty() && !config.pem.empty())
			{
				logger->AddOutput<custom::WeChatOutput>(config.wx, config.pem);
			}
			mongo::MongoConfig mongoConfig;
			if (!config.mongo.empty() && mongoConfig.FromString(config.mongo))
			{
				logger->AddOutput<custom::MongoOutput>(mongoConfig);
			}
		}
		if (!logger->Start())
		{
			return false;
		}
		this->mLoggers.emplace(config.name, logger);
		this->mLevelLoggers.emplace(config.level, logger);
		return true;
	}
}