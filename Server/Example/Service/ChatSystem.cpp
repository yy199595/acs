//
// Created by yy on 2023/9/10.
//

#include"ChatSystem.h"
#include"Util/Tools/TimeHelper.h"
#include"Gate/Service/GateSystem.h"
#include"Gate/Component/GateComponent.h"
#include "Entity/Component/ActorComponent.h"
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
		LOG_CHECK_RET_FALSE(this->mActor = this->GetComponent<ActorComponent>())
		return true;
	}

	bool ChatSystem::Awake()
	{

		FriendInfo::RegisterField("add_time", &FriendInfo::add_time);
		FriendInfo::RegisterField("friend_id", &FriendInfo::friend_id);
		FriendInfo::RegisterField("friend_list", &FriendInfo::friend_list);


		LoginInfo::RegisterField("ip", &LoginInfo::ip);
		LoginInfo::RegisterField("login_time", &LoginInfo::login_time);

		PlayerAccountInfo::RegisterField("item_list", &PlayerAccountInfo::item_list);
		PlayerAccountInfo::RegisterField("user_id", &PlayerAccountInfo::user_id);
		PlayerAccountInfo::RegisterField("account", &PlayerAccountInfo::account);
		PlayerAccountInfo::RegisterField("password", &PlayerAccountInfo::password);
		PlayerAccountInfo::RegisterField("create_time", &PlayerAccountInfo::create_time);
		PlayerAccountInfo::RegisterField("register_time", &PlayerAccountInfo::register_time);
		PlayerAccountInfo::RegisterField("login_info", &PlayerAccountInfo::login_info);
		PlayerAccountInfo::RegisterField("friend_list", &PlayerAccountInfo::friend_list);

		PlayerAccountInfo playerAccountInfo;
		{
			playerAccountInfo.user_id = 10000;
			playerAccountInfo.account = "yjz1995";
			playerAccountInfo.password = "199595yjz.";
			playerAccountInfo.create_time = help::Time::NowSec();
			playerAccountInfo.register_time = help::Time::NowMic();

			playerAccountInfo.item_list.emplace_back(100);
			playerAccountInfo.item_list.emplace_back(101);
			playerAccountInfo.item_list.emplace_back(103);

			playerAccountInfo.login_info.ip = "127.0.0.1";
			playerAccountInfo.login_info.login_time = help::Time::NowMil();

			FriendInfo friendInfo1;
			friendInfo1.friend_id = 10002;
			friendInfo1.add_time = help::Time::NowSec();

			FriendInfo friendInfo;
			friendInfo.friend_id = 10001;
			friendInfo.friend_list.emplace_back(friendInfo);
			friendInfo.friend_list.emplace_back(friendInfo1);

			friendInfo.add_time = help::Time::NowSec();
			playerAccountInfo.friend_list.emplace_back(friendInfo);
		}

		std::string json;
		playerAccountInfo.Encode(json);
		std::unique_ptr<PlayerAccountInfo> newPlayerAccountInfo = PlayerAccountInfo::Create(json);

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
		Player * player = this->mActor->GetPlayer(playerId);
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
				Player * targetPlayer = this->mActor->GetPlayer(targetId);
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