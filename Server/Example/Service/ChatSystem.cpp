//
// Created by yy on 2023/9/10.
//

#include"ChatSystem.h"
#include"Util/Tools/TimeHelper.h"
#include"Gate/Service/GateSystem.h"
#include"Gate/Component/GateComponent.h"

constexpr int CHAT_TYPE_WORLD = 1;
constexpr int CHAT_TYPE_PRIVATE = 2;
constexpr int CHAT_TYPE_GUILD = 3;

namespace acs
{
	bool ChatSystem::OnInit()
	{
		BIND_PLAYER_RPC_METHOD(ChatSystem::OnChat);
		LOG_CHECK_RET_FALSE(this->mGate = this->GetComponent<GateComponent>())
		return true;
	}

	int ChatSystem::OnChat(long long playerId, const c2s::chat::request& request)
	{
		long long lastTime = 0;
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
				this->mGate->BroadCast(func, &message);
				break;
			case CHAT_TYPE_GUILD:
				break;
			case CHAT_TYPE_PRIVATE:
				this->mGate->Send(request.user_id(), func, message);
				break;
			default:
				return XCode::CallArgsError;
		}
		this->mChatTime.Set(playerId, nowTime);
		return XCode::Ok;
	}

}