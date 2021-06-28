#include"NetWorkAction.h"
#include<Pool/ProtocolPool.h>
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
		ProtocolPool * pool = ProtocolPool::Get();
		const std::string &message = requestData->messagedata();
		const std::string &protocName = requestData->protocname();
		
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
	XCode LocalMysqlQueryActionProxy::Invoke(const shared_ptr<NetWorkPacket> requestData, shared_ptr<NetWorkPacket> returnData)
	{
		ProtocolPool * pool = ProtocolPool::Get();
		const std::string &message = requestData->messagedata();
		const std::string &protocName = requestData->protocname();
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
			returnData->set_messagedata(responseMessage);
		}
		pool->Destory(requestMessage);
		return code;
	}
}
