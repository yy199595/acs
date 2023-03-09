//
// Created by zmhy0073 on 2021/12/1.
//

#ifndef GAMEKEEPER_GATESERVICE_H
#define GAMEKEEPER_GATESERVICE_H
#include"Service/PhysicalService.h"
namespace Sentry
{
	class Gate final : public PhysicalService
	{
	 public:
		Gate();
	 private:
		int Ping(long long userId);		
        int Allocation(long long userId, s2s::allot::response & response);		
	private:
        void Init() final;
		bool OnStart() final;
        bool OnClose() final;
	 private:
		std::string mAddress;
		class NodeMgrComponent* mNodeComponent;
		class OuterNetMessageComponent* mOuterComponent;
	};

}

#endif //GAMEKEEPER_GATESERVICE_H
