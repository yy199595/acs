//
// Created by 64658 on 2024/12/27.
//

#include "PlayerSystem.h"
#include "Common/Component/PlayerComponent.h"
namespace acs
{
	PlayerSystem::PlayerSystem()
	{
		this->mPlayer = nullptr;
	}

	bool PlayerSystem::OnInit()
	{
		LOG_CHECK_RET_FALSE(this->mPlayer = this->GetComponent<PlayerComponent>())
		return true;
	}
}