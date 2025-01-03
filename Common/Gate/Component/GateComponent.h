//
// Created by leyi on 2023/9/11.
//

#ifndef APP_GATECOMPONENT_H
#define APP_GATECOMPONENT_H
#include "Rpc/Client/Message.h"
#include "Gate/Common/Common.h"
#include "Entity/Component/Component.h"
namespace acs
{
	class GateComponent final : public Component
	{
	public:
		GateComponent();
		~GateComponent() override = default;
	public:
		int OnMessage(rpc::Message * request);
		int Send(int id, rpc::Message * message);
		void Broadcast(rpc::Message * message);
	private:
		bool LateAwake() final;
		int OnRequest(rpc::Message * message);
		int OnResponse(rpc::Message * message);
	private:
		math::NumberPool<int> mNumPool;
		class ActorComponent * mActor;
		class RouterComponent * mRouter;
		std::unordered_map<char, IGate *> mGateComponents;
	};
}


#endif //APP_GATECOMPONENT_H
