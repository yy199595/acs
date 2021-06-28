#include "NetWorkAction.h"

namespace SoEasy
{
	XCode LocalActionProxy1::Invoke(shared_ptr<NetWorkPacket> requestData, shared_ptr<NetWorkPacket> returnData)
	{
		const long long operId = requestData->entityid();
		return this->mBindAction(operId);
	}
}

namespace SoEasy
{
	XCode LocalMysqlActionProxy::Invoke(const shared_ptr<NetWorkPacket> requestData, shared_ptr<NetWorkPacket> returnData)
	{
		const std::string &message = requestData->messagedata();
		const std::string &protocName = requestData->protocname();
		shared_ptr<Message> requestMessage = ObjectFactory::Get()->CreateShareMessage(protocName);
		if (requestMessage == nullptr)
		{
			return XCode::CreatePorotbufFail;
		}
		if (!requestMessage->ParseFromString(message))
		{
			return XCode::ParseMessageError;
		}
		return this->mBindAction(requestMessage);
	}
}

namespace SoEasy
{
	XCode LocalMysqlQueryActionProxy::Invoke(const shared_ptr<NetWorkPacket> requestData, shared_ptr<NetWorkPacket> returnData)
	{
		const std::string &message = requestData->messagedata();
		const std::string &protocName = requestData->protocname();
		shared_ptr<Message> requestMessage = ObjectFactory::Get()->CreateShareMessage(protocName);
		if (requestMessage == nullptr)
		{
			return XCode::CreatePorotbufFail;
		}
		if (!requestMessage->ParseFromString(message))
		{
			return XCode::ParseMessageError;
		}
		XCode code = this->mBindAction(requestMessage, requestMessage);
		if(code == XCode::Successful)
		{
			std::string responseMessage;
			if(!requestMessage->SerializePartialToString(&responseMessage))
			{
				return XCode::SerializationFailure;
			}
			returnData->set_messagedata(responseMessage);
		}
		return code;
	}
}
