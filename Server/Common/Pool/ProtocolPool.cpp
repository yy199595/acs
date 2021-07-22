#include "ProtocolPool.h"
#include<Define/CommonDef.h>
namespace Sentry
{
	ProtocolPool GprotocolPool;
	SharedMessage ProtocolPool::Create(const std::string & name)
	{
		if (name.empty())
		{
			return nullptr;
		}
		auto iter = this->mProtocolMap.find(name);
		if (iter != this->mProtocolMap.end())
		{
			std::queue<SharedMessage> & messageQueue = iter->second;
			if (!messageQueue.empty())
			{
				SharedMessage messageData = messageQueue.front();
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
		return pMessage != nullptr ? std::shared_ptr(pMessage->New()) : nullptr;
	}

	bool ProtocolPool::Destory(SharedMessage messageData)
	{
		if (messageData == nullptr)
		{
			return true;
		}
		const std::string & name = messageData->GetTypeName();
		auto iter = this->mProtocolMap.find(name);
		if (iter == this->mProtocolMap.end())
		{
			std::queue<SharedMessage> messageQueue;
			this->mProtocolMap.emplace(name, messageQueue);
		}
		std::queue<SharedMessage> & messageQueue = this->mProtocolMap[name];
		if (messageQueue.size() < ProtocolMaxCount)
		{
			messageData->Clear();
			messageQueue.push(messageData);
		}	
		return true;
	}
}