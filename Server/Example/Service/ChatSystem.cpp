//
// Created by yy on 2023/9/10.
//

#include"ChatSystem.h"
#include"Util/Tools/TimeHelper.h"
#include"Gate/Service/GateSystem.h"
#include "Common/Entity/Player.h"
#include "Common/Component/PlayerComponent.h"

constexpr int CHAT_TYPE_WORLD = 1;
constexpr int CHAT_TYPE_PRIVATE = 2;
constexpr int CHAT_TYPE_GUILD = 3;

namespace acs
{
	bool ChatSystem::OnInit()
	{
		BIND_RPC_METHOD(ChatSystem::OnChat);
		BIND_RPC_METHOD(ChatSystem::OnPing);
		LOG_CHECK_RET_FALSE(this->mPlayerMgr = this->GetComponent<PlayerComponent>())
		return true;
	}

	int ChatSystem::OnPing(long long playerId)
	{
		return XCode::Ok;
	}

	int ChatSystem::OnChat(long long playerId, const c2s::chat::request& request)
	{
		long long lastTime = 0;
		Player * player = this->mPlayerMgr->Get(playerId);
		if(player == nullptr)
		{
			return XCode::NotFindUser;
		}
		this->mChatTime.Get(playerId, lastTime);
		long long nowTime = help::Time::NowSec();
		if (nowTime - lastTime < 1)
		{
			return XCode::CallFrequently;
		}

		c2s::chat::notice message;
		message.set_message(request.message());
		message.set_msg_type(request.msg_type());
		const std::string func("ChatComponent.OnChat");
		switch (request.msg_type())
		{
			case CHAT_TYPE_WORLD:
				// TODO 广播
				break;
			case CHAT_TYPE_GUILD:
				break;
			case CHAT_TYPE_PRIVATE:
			{
				long long targetId = request.user_id();
				Player * targetPlayer = this->mPlayerMgr->Get(targetId);
				if(targetPlayer != nullptr)
				{
					targetPlayer->Send(func, message);
				}
				break;
			}
			default:
				return XCode::CallArgsError;
		}
		this->mChatTime.Set(playerId, nowTime);
		return XCode::Ok;
	}

}