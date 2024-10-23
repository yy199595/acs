//
// Created by yy on 2023/9/10.
//

#ifndef APP_CHATSYSTEM_H
#define APP_CHATSYSTEM_H
#include"Message/c2s/c2s.pb.h"
#include"Core/Map/HashMap.h"
#include"Rpc/Service/RpcService.h"
namespace acs
{
	class ChatSystem : public RpcService
	{
	private:
		bool OnInit() final;
	private:
		int OnChat(long long playerId, const c2s::chat::request & request);
	private:
		class GateComponent * mGate;
		custom::HashMap<long long, long long> mChatTime;
	};
}


#endif //APP_CHATSYSTEM_H
