//
// Created by 64658 on 2024/9/2.
//

#ifndef APP_PUBSUBSYSTEM_H
#define APP_PUBSUBSYSTEM_H
#include <queue>
#include "Rpc/Service/RpcService.h"

namespace sub
{
	struct Client
	{
	public:
		int nodeId = 0;
		int sockId = 0;
		std::unordered_set<std::string> channel;
		//std::queue<std::unique_ptr<rpc::Message>> messages;
	};
}

namespace acs
{
	class PubSubSystem final : public RpcService
	{
	public:
		PubSubSystem();
		~PubSubSystem() final = default;
	private:
		bool OnInit() final;
	private:
		int Subscribe(const rpc::Message & request);
		int UbSubscribe(const rpc::Message & request);
		int Publish(const json::r::Document & request);
	private:
		class RouterComponent * mRouter;
		std::unordered_map<int, sub::Client> mSubInfos;
	};
}


#endif //APP_PUBSUBSYSTEM_H
