#include "MessagePool.h"
namespace Sentry
{
	std::unordered_map<std::string, Message *> MessagePool::mMessageMap;
	Message * MessagePool::New(const std::string & name)
	{
		if (name.empty())
		{
			return nullptr;
		}
		auto iter = mMessageMap.find(name);
		if (iter != mMessageMap.end())
		{
			Message * message = iter->second;
			message->Clear();
			return message;
		}

		const DescriptorPool *pDescriptorPool = DescriptorPool::generated_pool();
		const Descriptor *pDescriptor = pDescriptorPool->FindMessageTypeByName(name);
		if (pDescriptor == nullptr)
		{
			return nullptr;
		}
		MessageFactory *factory = MessageFactory::generated_factory();
		const Message *pMessage = factory->GetPrototype(pDescriptor);
		Message * newMessage = pMessage->New();
		if (newMessage != nullptr)
		{
			mMessageMap.emplace(name, newMessage);
			return newMessage;
		}
		return nullptr;
	}

	Message * MessagePool::NewByJson(const std::string & name, const std::string & json)
	{
		Message * message = New(name);
		if (message == nullptr)
		{
			return nullptr;
		}
		return util::JsonStringToMessage(json, message).ok() ? message : nullptr;
	}

	Message * MessagePool::NewByJson(const std::string & name, const char * json, size_t size)
	{
		Message * message = New(name);
		if (message == nullptr)
		{
			return nullptr;
		}
		if (util::JsonStringToMessage(StringPiece(json, size), message).ok())
		{
			return message;
		}
		return nullptr;
	}

	Message * MessagePool::NewByData(const std::string & name, const std::string & data)
	{
		Message * message = New(name);
		if (message == nullptr)
		{
			return nullptr;
		}
		if (!message->ParseFromString(data))
		{
			return nullptr;
		}
		return message;
	}

	Message * MessagePool::NewByData(const std::string & name, const char * json, size_t size)
	{
		Message * message = New(name);
		if (message == nullptr)
		{
			return nullptr;
		}
		if (!message->ParseFromArray(json, size))
		{
			return nullptr;
		}
		return message;
	}

}
