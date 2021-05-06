#include "Command.h"
#include<CommonCore/Applocation.h>
namespace SoEasy
{
	XCode StopCommand::Invoke(const std::string & paramate, RapidJsonWriter & returnData)
	{
		Applocation::Get()->Stop();
		return XCode::Successful;
	}

	XCode StateCommand::Invoke(const std::string & paramate, RapidJsonWriter & returnData)
	{
		char buffer[1024] = { 0 };
		Applocation * app = Applocation::Get();
		std::stringstream nStringBuffer;
		nStringBuffer << "fps = " << app->GetFps() << "\n";
		nStringBuffer << "logic time = " << app->GetLogicTime() << "\n";
		nStringBuffer << "delatime = " << app->GetDelaTime();

		const std::string message = nStringBuffer.str();
		returnData.AddParameter("Message", message);
		return XCode::Successful;
	}
}
