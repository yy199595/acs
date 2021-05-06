#include "ObjectFactory.h"
#include<CommonDefine/CommonDef.h>

namespace SoEasy
{
	ObjectFactory * ObjectFactory::mObjectFactory = nullptr;


	bool ObjectFactory::Init(size_t size)
	{
		if (mObjectFactory == nullptr)
		{
			mObjectFactory = new ObjectFactory(size);
			if (mObjectFactory == nullptr)
			{
				return false;
			}
			return true;
		}
		return false;
	}


	Message * ObjectFactory::CreateMessage(const std::string & name)
	{
		if (name.empty())
		{
			return nullptr;
		}
		auto iter = mMessageMap.find(name);
		if (iter != mMessageMap.end())
		{
			Message * pMessage = iter->second;
			pMessage->Clear();
			return pMessage;
		}
		const DescriptorPool * pDescriptorPool = DescriptorPool::generated_pool();
		const Descriptor * pDescriptor = pDescriptorPool->FindMessageTypeByName(name);
		if (pDescriptor != nullptr)
		{
			MessageFactory * factory = MessageFactory::generated_factory();
			Message * pMessage = const_cast<Message *>(factory->GetPrototype(pDescriptor));
			mMessageMap.insert(std::make_pair(name, pMessage));
			return pMessage;
		}
		return nullptr;
	}
}
