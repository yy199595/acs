#include"Command.h"
#include<Core/Applocation.h>
#include<Service/ServiceBase.h>
namespace SoEasy
{
	CommandBase::CommandBase(CommandManager * commandMgr)
	{
		this->mCommandManager = commandMgr;
	}

	void StateCommand::Invoke(SharedTelnetSession session, const std::string paramate)
	{

	}

	void ServiceListCommand::Invoke(SharedTelnetSession session, const std::string paramate)
	{
		std::stringstream returnMessage;
		std::vector<ServiceBase *> services;
		Applocation::Get()->GetServices(services);
		for (ServiceBase * service : services)
		{
			returnMessage << service->GetTypeName() << '\n';
		}
		const std::string & address = session->GetAddress();
		this->mCommandManager->AddCommandBackArgv(address, XCode::Successful, returnMessage.str());
	}
}
