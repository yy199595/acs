//
// Created by yy on 2023/9/10.
//

#include"ChatSystem.h"
#include"Util/Tools/TimeHelper.h"
#include"Gate/Service/GateSystem.h"
#include"Gate/Component/GateComponent.h"

#include "Util/Tools/String.h"
constexpr int CHAT_TYPE_WORLD = 1;
constexpr int CHAT_TYPE_PRIVATE = 2;
constexpr int CHAT_TYPE_GUILD = 3;

namespace acs
{
	bool ChatSystem::OnInit()
	{
		BIND_PLAYER_RPC_METHOD(ChatSystem::OnChat);
		BIND_PLAYER_RPC_METHOD(ChatSystem::OnPing);
		LOG_CHECK_RET_FALSE(this->mGate = this->GetComponent<GateComponent>())
		return true;
	}

	bool ChatSystem::Awake()
	{
		FriendInfo::RegisterField("add_time", &FriendInfo::add_time);
		FriendInfo::RegisterField("friend_id", &FriendInfo::friend_id);

		LoginInfo::RegisterField("ip", &LoginInfo::ip);
		LoginInfo::RegisterField("login_time", &LoginInfo::login_time);


		PlayerAccountInfo::SetName("player_account");
		PlayerAccountInfo::RegisterField("user_id", &PlayerAccountInfo::user_id);
		PlayerAccountInfo::RegisterField("account", &PlayerAccountInfo::account);
		PlayerAccountInfo::RegisterField("password", &PlayerAccountInfo::password);
		PlayerAccountInfo::RegisterField("create_time", &PlayerAccountInfo::create_time);
		PlayerAccountInfo::RegisterField("register_time", &PlayerAccountInfo::register_time);
		PlayerAccountInfo::RegisterObject("login_info", &PlayerAccountInfo::login_info);
		PlayerAccountInfo::RegisterField("friend_list", &PlayerAccountInfo::friend_list);

		PlayerAccountInfo playerAccountInfo;
		{
			playerAccountInfo.user_id = 10000;
			playerAccountInfo.account = "yjz1995";
			playerAccountInfo.password = "199595yjz.";
			playerAccountInfo.create_time = help::Time::NowSec();
			playerAccountInfo.register_time = help::Time::NowMic();

			playerAccountInfo.login_info.ip = "127.0.0.1";
			playerAccountInfo.login_info.login_time = help::Time::NowMil();

			FriendInfo friendInfo;
			friendInfo.friend_id = 10001;
			friendInfo.add_time = help::Time::NowSec();
			playerAccountInfo.friend_list.emplace_back(friendInfo);
		}

		std::string json;
		playerAccountInfo.Encode(json);
		PlayerAccountInfo newPlayerAccountInfo;

		newPlayerAccountInfo.Decode(json);
		LOG_INFO("\n{}", help::Str::FormatJson(json))
		return true;
	}

	int ChatSystem::OnPing(long long playerId)
	{
		return XCode::Ok;
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