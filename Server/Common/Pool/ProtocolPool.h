#pragma once
#include<queue>
#include<unordered_map>
#include<meory>
#include<google/protobuf/message.h>
using namespace google::protobuf;
#define ProtocolMaxCount 10
namespace Sentry
{
	typename SharedMessage std::shared_ptr<Message>;
	class ProtocolPool
	{
	public:
		SharedMessage Create(const std::string & name);
		bool Destory(SharedMessage messageData);
	private:
		std::unordered_map<std::string, std::queue<SharedMessage>> mProtocolMap;
	};
	extern ProtocolPool GprotocolPool;
}