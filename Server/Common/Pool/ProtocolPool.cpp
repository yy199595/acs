#include "ProtocolPool.h"
#include<Define/CommonDef.h>
namespace SoEasy
{
	ProtocolPool GprotocolPool;
	Message * ProtocolPool::Create(const std::string & name)
	{
		if (name.empty())
		{
			return nullptr;
		}
		auto iter = this->mProtocolMap.find(name);
		if (iter != this->mProtocolMap.end())
		{
			std::queue<Message *> messageQueue = iter->second;
			if (!messageQueue.empty())
			{
				Message * messageData = messageQueue.front();
				messageQueue.pop();
				return messageData;
			}
		}

		const DescriptorPool *pDescriptorPool = DescriptorPool::generated_pool();
		const Descriptor *pDescriptor = pDescriptorPool->FindMessageTypeByName(name);
		if (pDescriptor == nullptr)
		{
			SayNoDebugError("create pb fail " << name);
			return nullptr;
		}
		MessageFactory *factory = MessageFactory::generated_factory();
		const Message *pMessage = factory->GetPrototype(pDescriptor);
		return pMessage != nullptr ? pMessage->New() : nullptr;
	}

	bool ProtocolPool::Destory(Message * messageData)
	{
		if (messageData == nullptr)
		{
			return true;
		}
		const std::string & name = messageData->GetTypeName();
		auto iter = this->mProtocolMap.find(name);
		if (iter == this->mProtocolMap.end())
		{
			std::queue<Message *> messageQueue;
			this->mProtocolMap.emplace(name, messageQueue);
		}
		std::queue<Message *> messageQueue = this->mProtocolMap[name];
		if (messageQueue.size() >= ProtocolMaxCount)
		{
			delete messageData;
			return true;
		}
		messageData->Clear();
		messageQueue.push(messageData);
		return true;
	}
}