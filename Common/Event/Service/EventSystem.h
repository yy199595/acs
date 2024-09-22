//
// Created by 64658 on 2024/9/2.
//

#ifndef APP_EVENTSYSTEM_H
#define APP_EVENTSYSTEM_H
#include "Rpc/Service/RpcService.h"
namespace acs
{
	class EventSystem final : public RpcService
	{
	public:
		EventSystem();
		~EventSystem() final = default;
	private:
		bool OnInit() final;
	private:
		int Publish(const rpc::Packet & request);
	private:
		class DispatchComponent * mDispatch;
	};
}


#endif //APP_EVENTSYSTEM_H
