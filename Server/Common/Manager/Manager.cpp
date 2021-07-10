#include"Manager.h"
#include"ActionManager.h"
#include<Core/Applocation.h>
#include<Thread/ThreadTaskAction.h>

namespace SoEasy
{

	
	NetMessageBuffer::NetMessageBuffer(const std::string & address, const SharedPacket packet)
		:mAddress(address), mMessagePacket(packet)
	{

	}
	
	void Manager::ForeachManagers(std::function<bool(Manager*)> action)
	{
		std::vector<Manager *> managers;
		this->GetApp()->GetManagers(managers);
		for (size_t index = 0; index < managers.size(); index++)
		{
			Manager * manager = managers[index];
			if (action(manager) == false)
			{
				break;
			}
		}
	}
}