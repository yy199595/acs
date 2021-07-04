#include"NetWorkAction.h"
#include<Pool/ProtocolPool.h>
namespace SoEasy
{
	XCode LocalActionProxy1::Invoke(PB::NetWorkPacket * messageData)
	{
		const long long operId = messageData->entityid();
		return this->mBindAction(operId);
	}
}

namespace SoEasy
{
	XCode LocalMysqlActionProxy::Invoke(PB::NetWorkPacket * messageData)
	{
		ProtocolPool * pool = ProtocolPool::Get();
		const std::string &message = messageData->messagedata();
		const std::string &protocName = messageData->protocname();
		
		Message * requestMessage = pool->Create(protocName);
		if (requestMessage == nullptr)
		{
			return XCode::CreatePorotbufFail;
		}
		if (!requestMessage->ParseFromString(message))
		{
			pool->Destory(requestMessage);
			return XCode::ParseMessageError;
		}
		XCode code = this->mBindAction(*requestMessage);
		pool->Destory(requestMessage);
		return code;
	}
}

namespace SoEasy
{
	XCode LocalMysqlQueryActionProxy::Invoke(PB::NetWorkPacket * messageData)
	{
		ProtocolPool * pool = ProtocolPool::Get();
		const std::string &message = messageData->messagedata();
		const std::string &protocName = messageData->protocname();
		Message * requestMessage = pool->Create(protocName);
		if (requestMessage == nullptr)
		{
			return XCode::CreatePorotbufFail;
		}
		if (!requestMessage->ParseFromString(message))
		{
			pool->Destory(requestMessage);
			return XCode::ParseMessageError;
		}
		XCode code = this->mBindAction(*requestMessage, *requestMessage);
		if(code == XCode::Successful)
		{
			std::string responseMessage;
			if(!requestMessage->SerializePartialToString(&responseMessage))
			{
				pool->Destory(requestMessage);
				return XCode::SerializationFailure;
			}
			messageData->set_messagedata(responseMessage);
		}
		pool->Destory(requestMessage);
		return code;
	}
}
