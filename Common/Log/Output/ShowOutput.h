//
// Created by yy on 2023/8/12.
//

#ifndef APP_SHOWOUTPUT_H
#define APP_SHOWOUTPUT_H
#include"Log/Common/Logger.h"
namespace custom
{
	class ShowOutput : public IOutput
	{
	public:
		ShowOutput() = default;
	private:
		void Flush() final { };
		void Push(Asio::Context &io, const std::string &name, const custom::LogInfo &logInfo) final;
	};
}


#endif //APP_SHOWOUTPUT_H
