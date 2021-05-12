#pragma once
#include"Component.h"
#include<Manager/NetWorkManager.h>
#include<Manager/LocalActionManager.h>
#include<Manager/RemoteActionManager.h>

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
		LocalActionManager * mLocalActionManager;
		RemoteActionManager * mRemoteActionManager;
		std::unordered_map<std::string, std::string> mActionAddress;
	};
}