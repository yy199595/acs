//
// Created by yy on 2023/8/12.
//

#ifndef APP_CONSOLEOUTPUT_H
#define APP_CONSOLEOUTPUT_H
#include"Log/Common/Logger.h"
namespace custom
{
	class ConsoleOutput : public IOutput
	{
	public:
		ConsoleOutput() = default;
	private:
		void Flush() final { };
		void Push(Asio::Context &io, const std::string &name, const custom::LogInfo &logInfo) final;
	};
}


#endif //APP_CONSOLEOUTPUT_H
