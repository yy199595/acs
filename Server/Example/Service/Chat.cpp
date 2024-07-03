//
// Created by yy on 2023/9/10.
//

#include"Chat.h"
#include"Gate/Service/Gate.h"
#include"Util/Time/TimeHelper.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Gate/Component/GateComponent.h"
#include"Redis/Component/RedisComponent.h"

namespace joke
{
	bool Chat::OnInit()
	{
		BIND_PLAYER_RPC_METHOD(Chat::OnChat);
		this->mGate = this->GetComponent<GateComponent>();
		this->mRedis = this->GetComponent<RedisComponent>();
		return true;
	}

	int Chat::OnChat(long long playerId, const c2s::chat::request& request)
	{
		const std::string hash("player.chat");
		redis::Response * response = this->mRedis->Run("HGET", hash, playerId);
		if(response == nullptr || !response->IsString())
		{
			return XCode::Failure;
		}
		long long lastChatTime = 0;
		if(!response->GetString().empty())
		{
			lastChatTime = std::stoll(response->GetString());
		}
		long long nowTime = help::Time::NowSec();
		if(nowTime - lastChatTime < 1)
		{
			return XCode::CallFrequently;
		}
		c2s::chat::notice message;
		message.set_user_id(playerId);
		message.set_message(request.message());
		message.set_msg_type(request.msg_type());
		this->mGate->BroadCast("ChatComponent.OnChat", &message);

		this->mRedis->Run("HSET", hash, playerId, nowTime);
		return XCode::Ok;
	}

}