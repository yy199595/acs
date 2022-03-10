#include "MessagePool.h"
namespace Helper
{
	std::unordered_map<std::string, const Message *> Proto::mMessageMap;
	std::shared_ptr<Message> Proto::New(const Any & any)
    {
        std::string fullName;
        if(!google::protobuf::Any::ParseAnyTypeUrl(any.type_url(), &fullName))
        {
            return nullptr;
        }
        return Proto::New(fullName);
    }

	std::shared_ptr<Message> Proto::NewByData(const Any &any)
    {
        std::string fullName;
        if (!google::protobuf::Any::ParseAnyTypeUrl(any.type_url(), &fullName))
        {
            return nullptr;
        }
		std::shared_ptr<Message> message = Proto::New(fullName);
        if (message != nullptr && any.UnpackTo(message.get()))
        {
            return message;
        }
        return nullptr;
    }

	std::shared_ptr<Message> Proto::NewByJson(const Any &any, const std::string &json)
    {
		std::shared_ptr<Message> message = Proto::New(any);
        if(message != nullptr && util::JsonStringToMessage(json, message.get()).ok())
        {
            return message;
        }
        return nullptr;
    }

	std::shared_ptr<Message> Proto::New(const std::string & name)
	{
		if (name.empty())
		{
			return nullptr;
		}
		auto iter = mMessageMap.find(name);
		if (iter != mMessageMap.end())
		{
			const Message* message = iter->second;
			return std::shared_ptr<Message>(message->New());
		}

		const DescriptorPool* pDescriptorPool = DescriptorPool::generated_pool();
		const Descriptor* pDescriptor = pDescriptorPool->FindMessageTypeByName(name);
		if (pDescriptor == nullptr)
		{
			return nullptr;
		}
		MessageFactory* factory = MessageFactory::generated_factory();
		const Message* pMessage = factory->GetPrototype(pDescriptor);
		if (pMessage != nullptr)
		{
			mMessageMap.emplace(name, pMessage);
			return std::shared_ptr<Message>(pMessage->New());
		}
		return nullptr;
	}

	std::shared_ptr<Message> Proto::NewByJson(const std::string & name, const std::string & json)
	{
		std::shared_ptr<Message> message = New(name);
		if (message == nullptr && util::JsonStringToMessage(json, message.get()).ok())
		{
			return message;
		}
		return nullptr;
	}

	std::shared_ptr<Message> Proto::NewByJson(const std::string & name, const char * json, size_t size)
	{
		std::shared_ptr<Message> message = New(name);
		if (message != nullptr &&util::JsonStringToMessage(
			StringPiece(json, size), message.get()).ok())
		{
			return message;
		}
		return nullptr;
	}

	bool Proto::GetJson(std::shared_ptr<Message> message, std::string& json)
	{
		json.clear();
		return util::MessageToJsonString(*message, &json).ok();
	}

    bool Proto::GetJson(const Any &message, std::string &json)
    {
        json.clear();
		std::shared_ptr<Message> pb = Proto::NewByData(message);
        if(pb == nullptr)
        {
            return false;
        }
        return message.UnpackTo(pb.get()) && util::MessageToJsonString(*pb, &json).ok();
    }

    bool Proto::GetJson(const Message &message, std::string &json)
    {
        return util::MessageToJsonString(message, &json).ok();
    }
}
