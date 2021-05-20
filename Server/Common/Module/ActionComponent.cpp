#include"ActionComponent.h"
namespace SoEasy
{
	ActionComponent::ActionComponent(SharedGameObject gameObject)
		:Component(gameObject)
	{

	}

	void ActionComponent::Call(const std::string & name, shared_ptr<NetWorkPacket> messageData)
	{
		
	}

	void ActionComponent::OnInit()
	{
		this->mLocalActionManager = this->GetManager<ActionManager>();
		this->mRemoteActionManager = this->GetManager<ServiceQuery>();
	}
}
