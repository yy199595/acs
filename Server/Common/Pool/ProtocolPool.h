#pragma once
#include<queue>
#include<unordered_map>
#include<google/protobuf/message.h>
using namespace google::protobuf;
#define ProtocolMaxCount 10
namespace Sentry
{
	class ProtocolPool
	{
	public:
		Message * Create(const std::string & name);
		bool Destory(Message * messageData);
	private:
		std::unordered_map<std::string, std::queue<Message *>> mProtocolMap;
	};
	extern ProtocolPool GprotocolPool;
}