//
// Created by yy on 2023/8/13.
//

#include"MongoOutput.h"
#include"Util/Time/TimeHelper.h"
#include"Mongo/Client/MongoFactory.h"

namespace custom
{
	MongoOutput::MongoOutput(mongo::MongoConfig  config)
		: mConfig(std::move(config))
	{

	}

	bool MongoOutput::Start(Asio::Context& io)
	{
		tcp::Socket * socket = new tcp::Socket(io);
		{
			socket->Init(this->mConfig.Address);
			this->mCommand = fmt::format("{}.$cmd", this->mConfig.DB);
			this->mMonClient = std::make_unique<mongo::Client>(socket, this->mConfig);
		}
		return this->mMonClient->Start(false);
	}

	void MongoOutput::Push(Asio::Context &io, const std::string& name, const custom::LogInfo& logInfo)
	{
		std::string time = help::Time::GetDateString();

		bson::Writer::Document document;
		if(!logInfo.File.empty())
		{
			document.Add("file", logInfo.File);
		}
		if(!logInfo.Stack.empty())
		{
			document.Add("stack", logInfo.Stack);
		}
		document.Add("time", time);
		if(!document.FromByJson(logInfo.Content))
		{
			document.Add("log", logInfo.Content);
		}

		switch(logInfo.Level)
		{
			case custom::LogLevel::Info:
				document.Add("level", "info");
				break;
			case custom::LogLevel::Debug:
				document.Add("level", "debug");
				break;
			case custom::LogLevel::Warn:
				document.Add("level", "warn");
				break;
			case custom::LogLevel::Error:
				document.Add("level", "error");
				break;
			case custom::LogLevel::Fatal:
				document.Add("level", "fatal");
				break;
		}

		std::unique_ptr<mongo::Request> request = mongo::MongoFactory::Insert(name, document);
		{
			request->dataBase = this->mConfig.DB;
			request->collectionName = this->mCommand;
		}
		this->mMonClient->SendMongoCommand(std::move(request));
	}
}