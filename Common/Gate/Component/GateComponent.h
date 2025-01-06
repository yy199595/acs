//
// Created by leyi on 2023/9/11.
//

#ifndef APP_GATECOMPONENT_H
#define APP_GATECOMPONENT_H
#include "Rpc/Common/Message.h"
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
		class ActorComponent * mActor;
		math::NumberPool<int> mNumPool;
		class RouterComponent * mRouter;
		rpc::IOuterSender * mOuterComponent;
	};
}


#endif //APP_GATECOMPONENT_H
