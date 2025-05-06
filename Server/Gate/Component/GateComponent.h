//
// Created by leyi on 2023/9/11.
//

#ifndef APP_GATECOMPONENT_H
#define APP_GATECOMPONENT_H
#include "Rpc/Common/Message.h"
#include "Entity/Component/Component.h"
namespace acs
{
	class GateComponent final : public Component, public rpc::IOuterMessage
	{
	public:
		GateComponent();
		~GateComponent() override = default;
	public:
		int Send(int id, rpc::Message * message);
		void Broadcast(rpc::Message * message);
		int OnMessage(rpc::Message * request) noexcept final;
	private:
		bool LateAwake() final;
		int OnRequest(rpc::Message * message);
		int OnResponse(rpc::Message * message);
	private:
		class NodeComponent * mNode;
		math::NumberPool<int> mNumPool;
		class RouterComponent * mRouter;
		rpc::IOuterSender * mOuterComponent;
		class PlayerComponent * mPlayerComponent;
	};
}


#endif //APP_GATECOMPONENT_H
