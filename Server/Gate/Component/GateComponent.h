//
// Created by leyi on 2023/9/11.
//

#ifndef APP_GATECOMPONENT_H
#define APP_GATECOMPONENT_H
#include"Proto/Include/Message.h"
#include"Entity/Component/Component.h"
namespace acs
{
	class GateComponent final : public Component
	{
	public:
		GateComponent();
		~GateComponent() override = default;
	public:
		void BroadCast(const std::string & func, const pb::Message * message = nullptr);
	private:
		bool LateAwake() final;
	private:
		std::string mGateName;
		class ActorComponent * mActor;
		std::vector<int> mGateServers;
	};
}


#endif //APP_GATECOMPONENT_H
