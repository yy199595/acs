//
// Created by yy on 2023/9/10.
//

#ifndef APP_CHATSYSTEM_H
#define APP_CHATSYSTEM_H
#include"Message/c2s/c2s.pb.h"
#include"Core/Map/HashMap.h"
#include"Rpc/Service/RpcService.h"
#include "Yyjson/Object/JsonObject.h"
namespace acs
{
	struct LoginInfo : public json::Object<LoginInfo>
	{
		std::string ip;
		long long login_time;
	};

	struct FriendInfo : public json::Object<FriendInfo>
	{
		long long friend_id;
		long long add_time;
	};

	struct PlayerAccountInfo : public json::Object<PlayerAccountInfo>
	{
		long long user_id;
		std::string account;
		std::string password;
		long long create_time;
		long long register_time;
		LoginInfo login_info;
		std::vector<int> item_list;
		std::vector<FriendInfo> friend_list;
	};
}

namespace acs
{
	class ChatSystem : public RpcService
	{
	private:
		bool Awake() final;
		bool OnInit() final;
	private:
		int OnPing(long long playerId);
		int OnChat(long long playerId, const c2s::chat::request & request);
	private:
		class GateComponent * mGate;
		custom::HashMap<long long, long long> mChatTime;
	};
}


#endif //APP_CHATSYSTEM_H
