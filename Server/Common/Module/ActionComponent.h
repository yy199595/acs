#pragma once
#include"Component.h"
#include<Manager/NetWorkManager.h>
#include<Manager/ActionManager.h>

namespace SoEasy
{
	class ActionComponent : public Component
	{
	public:
		ActionComponent(SharedGameObject gameObject);
		~ActionComponent() { }
	public:
		void Call(const std::string & name, shared_ptr<NetWorkPacket> messageData);
	protected:
		void OnInit() override;
	private:
		NetWorkManager * mNetWorkManager;
		ActionManager * mLocalActionManager;
		std::unordered_map<std::string, std::string> mActionAddress;
	};
}