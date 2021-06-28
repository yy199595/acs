#include "ObjectFactory.h"
#include <Define/CommonDef.h>

namespace SoEasy
{
	ObjectFactory *ObjectFactory::Get()
	{
		static ObjectFactory factory(10);
		return &factory;
	}

	Message *ObjectFactory::CreateMessage(const std::string &name)
	{
		if (name.empty())
		{
			return nullptr;
		}
		auto iter = mMessageMap.find(name);
		if (iter != mMessageMap.end())
		{
			Message *pMessage = iter->second;
			pMessage->Clear();
			return pMessage;
		}
		const DescriptorPool *pDescriptorPool = DescriptorPool::generated_pool();
		const Descriptor *pDescriptor = pDescriptorPool->FindMessageTypeByName(name);
		if (pDescriptor != nullptr)
		{
			MessageFactory *factory = MessageFactory::generated_factory();
			const Message *pMessage = factory->GetPrototype(pDescriptor);
			if (pMessage != nullptr)
			{
				Message *newMessage = pMessage->New();
				mMessageMap.insert(std::make_pair(name, newMessage));
				return newMessage;
			}
		}
		return nullptr;
	}

	shared_ptr<Message> ObjectFactory::CreateShareMessage(const std::string &name)
	{
		if (name.empty())
		{
			return nullptr;
		}
		const DescriptorPool *pDescriptorPool = DescriptorPool::generated_pool();
		const Descriptor *pDescriptor = pDescriptorPool->FindMessageTypeByName(name);
		if (pDescriptor != nullptr)
		{
			MessageFactory *factory = MessageFactory::generated_factory();
			const Message *pMessage = factory->GetPrototype(pDescriptor);
			if (pMessage != nullptr)
			{
				Message *newMessage = pMessage->New();
				return shared_ptr<Message>(newMessage);
			}
		}
		return nullptr;
	}
}
