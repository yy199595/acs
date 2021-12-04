#include "MessagePool.h"
namespace GameKeeper
{
	std::unordered_map<std::string, Message *> MessagePool::mMessageMap;

	Message * MessagePool::New(const Any & any)
    {
        std::string fullName;
        if(!google::protobuf::Any::ParseAnyTypeUrl(any.type_url(), &fullName))
        {
            return nullptr;
        }
        return MessagePool::New(fullName);
    }

    Message *MessagePool::NewByData(const Any &any, bool clone)
    {
        std::string fullName;
        if (!google::protobuf::Any::ParseAnyTypeUrl(any.type_url(), &fullName))
        {
            return nullptr;
        }
        Message *message = MessagePool::New(fullName);
        if (message != nullptr && any.UnpackTo(message))
        {
            if(clone)
            {
                auto obj = message->New();
                obj->CopyFrom(*message);
                return obj;
            }
            return message;
        }
        return nullptr;
    }

    Message *MessagePool::NewByJson(const Any &any, const std::string &json,bool clone)
    {
        Message * message = MessagePool::New(any);
        if(message == nullptr)
        {
            return nullptr;
        }
        if(util::JsonStringToMessage(json, message).ok())
        {
            if(clone)
            {
                auto obj = message->New();
                obj->CopyFrom(*message);
                return obj;
            }
            return message;
        }
        return nullptr;
    }

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

	Message * MessagePool::NewByJson(const std::string & name, const std::string & json,bool clone)
	{
		Message * message = New(name);
		if (message == nullptr)
		{
			return nullptr;
		}
        if(util::JsonStringToMessage(json, message).ok())
        {
            if(clone)
            {
                auto obj = message->New();
                obj->CopyFrom(*message);
                return obj;
            }
            return message;
        }
		return nullptr;
	}

	Message * MessagePool::NewByJson(const std::string & name, const char * json, size_t size,bool clone)
	{
		Message * message = New(name);
		if (message == nullptr)
		{
			return nullptr;
		}
		if (util::JsonStringToMessage(StringPiece(json, size), message).ok())
		{
            if(clone)
            {
                auto obj = message->New();
                obj->CopyFrom(*message);
                return obj;
            }
			return message;
		}
		return nullptr;
	}

	Message * MessagePool::NewByData(const std::string & name, const std::string & data,bool clone)
	{
		Message * message = New(name);
		if (message == nullptr)
		{
			return nullptr;
		}
		if (message->ParseFromString(data))
		{
            if(clone)
            {
                auto obj = message->New();
                obj->CopyFrom(*message);
                return obj;
            }
            return message;
		}
        return nullptr;
	}

	Message * MessagePool::NewByData(const std::string & name, const char * json, size_t size,bool clone)
    {
        Message *message = New(name);
        if (message == nullptr)
        {
            return nullptr;
        }
        if (message->ParseFromArray(json, size))
        {
            if(clone)
            {
                auto obj = message->New();
                obj->CopyFrom(*message);
                return obj;
            }
            return message;
        }
        return nullptr;
    }

}
