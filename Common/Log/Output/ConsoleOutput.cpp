//
// Created by yy on 2023/8/12.
//
#include "Log/Common/Debug.h"
#include "ConsoleOutput.h"
#include "Util/Tools/TimeHelper.h"

namespace custom
{
	void ConsoleOutput::Push(Asio::Context &io, const std::string& name, const custom::LogInfo& logInfo)
	{
		Debug::Console(logInfo);
	}
}