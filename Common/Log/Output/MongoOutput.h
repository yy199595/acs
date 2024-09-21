//
// Created by yy on 2023/8/13.
//

#ifndef APP_MONGOOUTPUT_H
#define APP_MONGOOUTPUT_H
#include"Log/Common/Logger.h"
#include"Mongo/Client/MongoClient.h"
namespace custom
{
	class MongoOutput : public IOutput
	{
	public:
		explicit MongoOutput(mongo::MongoConfig config);
	private:
		bool Start(Asio::Context &io) final;
		void Push(Asio::Context &io, const std::string &name, const custom::LogInfo &logInfo) final;
	private:
		std::string mCommand;
		mongo::MongoConfig mConfig;
		std::unique_ptr<mongo::Client> mMonClient;
	};
}


#endif //APP_MONGOOUTPUT_H
