
#include"ProtocHelper.h"
#include<google/protobuf/util/json_util.h>
using namespace google::protobuf::util;
namespace ProtocHelper
{
	bool GetJsonString(const Message & message, std::string & jsonString)
	{
		jsonString.clear();
		return MessageToJsonString(message, &jsonString).ok();
	}

	bool GetJsonString(const Message * message, std::string & jsonString)
	{
		jsonString.clear();
		return MessageToJsonString(*message, &jsonString).ok();
	}

	bool GetProtocObject(const std::string & jsonString, Message & message)
	{
		return JsonStringToMessage(jsonString, &message).ok();
	}

	Message * Create(const std::string name)
	{
		const DescriptorPool * pDescriptorPool = DescriptorPool::generated_pool();
		const Descriptor * pDescriptor = pDescriptorPool->FindMessageTypeByName(name);
		if (pDescriptor != nullptr)
		{
			MessageFactory * factory = MessageFactory::generated_factory();
			return const_cast<Message*>(factory->GetPrototype(pDescriptor));
		}
		return nullptr;
	}

	Message * Create(const std::string name, const std::string & jsonString)
	{
		Message * pMessage = Create(name);
		if (pMessage != nullptr && GetProtocObject(jsonString, *pMessage))
		{
			return pMessage;
		}
		delete pMessage;
		return nullptr;
	}
}