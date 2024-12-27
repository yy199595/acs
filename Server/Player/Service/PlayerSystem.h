//
// Created by 64658 on 2024/12/27.
//

#ifndef APP_PLAYERSYSTEM_H
#define APP_PLAYERSYSTEM_H
#include "Rpc/Service/RpcService.h"


namespace acs
{
	class PlayerSystem : public RpcService
	{
	public:
		PlayerSystem() = default;

	private:
		bool OnInit() final;
	private:
	};
}


#endif //APP_PLAYERSYSTEM_H
