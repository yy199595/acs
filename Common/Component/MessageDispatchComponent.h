#pragma once
#include "Component.h"
namespace Sentry
{
	class GameObject;
	class PacketMapper;
	class MessageDispatchComponent : public Component
	{
	public:
		MessageDispatchComponent();
		~MessageDispatchComponent();
	public:
		bool Awake() final;
		void AddHandleMessage(PacketMapper * message);
	private:
		void HandleMessage();
	private:

		bool mIsStarted;
		unsigned int mCorId;
		GameObject & mServiceObject;
		class CoroutineComponent * mCorComponent;
		std::queue<PacketMapper *> mWaitMsgQueue;
	};
}