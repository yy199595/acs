#include"NetMessageProxy.h"
#include<Manager/ProtocolManager.h>

namespace Sentry 
{
	template<typename T>
	bool MemoryCopy(T & tar, const void * sou, size_t & offset, const size_t maxsize)
	{
		memcpy(tar, sou + offset, sizeof(T);
		offset += sizeof(T);
		return offset <= maxsize;
	}


	NetMessageProxy::NetMessageProxy(NetMessageType type)
		:mMsgType(type)
	{
		this->mRpcId = 0;
		this->mUserId = 0;
	}
	NetMessageProxy * NetMessageProxy::Create(const char * message, const size_t size)
	{
		if (message == nullptr || size == 0)
		{
			return nullptr;
		}
		size_t offset = 1;
		Applocation * app = Applocation::Get();
		ProtocolManager * pProtocolMgr = app->GetManager<ProtocolManager>();

		int code = 0;
		long long rpcid = 0;
		long long userid = 0;
		unsigned short actionid = 0;
		const ProtocolConfig * config = nullptr;

		NetMessageType messageType = (NetMessageType)message[0];

		SayNoAssertRetNull_F(MemoryCopy(actionid, message, offset, size));
		config = pProtocolMgr->GetProtocolConfig(actionid);
		SayNoAssertRetNull_F(config);

		switch (messageType)
		{
		case s2sRequest:
			SayNoAssertRetNull_F(MemoryCopy(rpcid, message, offset, size));
			break;
		case s2sResponse:
			SayNoAssertRetNull_F(MemoryCopy(code, message, offset, size));
			SayNoAssertRetNull_F(MemoryCopy(rpcid, message, offset, size));
			break;
		case s2sNotice:

			break;
		case c2sRequest:
			SayNoAssertRetNull_F(MemoryCopy(rpcid, message, offset, size));
			SayNoAssertRetNull_F(MemoryCopy(userid, message, offset, size));
			break;
		case c2sResponse:
			SayNoAssertRetNull_F(MemoryCopy(rpcid, message, offset, size));
			SayNoAssertRetNull_F(MemoryCopy(userid, message, offset, size));
			SayNoAssertRetNull_F(MemoryCopy(code, message, offset, size));
			break;
		case c2sNotice:
		case s2cNotice:
			SayNoAssertRetNull_F(MemoryCopy(userid, message, offset, size));
		default:
			return nullptr;
		}

		NetMessageProxy * messageData = new NetMessageProxy(messageType);

		messageData->mRpcId = rpcid;
		messageData->mUserId = userid;
		messageData->mActionId = actionid;
		const std::string & request = config->RequestMsgName;
		const std::string & response = config->ResponseMsgName;

		const char * msg = message + offset;
		const size_t lenght = size - offset;
		if (messageType < RequestEnd)
		{
			messageData->mMethod = config->MethodName;
			messageData->mService = config->ServiceName;
			messageData->mReqMessage = pProtocolMgr->CreateMessage(request);
			if (messageData->mReqMessage != nullptr)
			{
				if (!messageData->mReqMessage->ParseFromArray(msg, lenght))
				{
					delete messageData;
					SayNoDebugError("parse request " << request << " msg error");
					return nullptr;
				}
				return messageData;
			}

			messageData->mJsonString.append(msg, lenght);
			return messageData;
		}

		if (messageType > RequestEnd)
		{
			messageData->mCode = (XCode)code;
			if (messageData->mCode != XCode::Successful)
			{
				return messageData;
			}

			messageData->mResMessage = pProtocolMgr->CreateMessage(response);
			if (messageData->mResMessage != nullptr)
			{
				if (!messageData->mResMessage->ParseFromArray(msg, lenght))
				{			
					SayNoDebugError("parse response " << response << " msg error");
					return nullptr;
				}
				return messageData;
			}
			messageData->mJsonString.append(msg, lenght);
			return messageData;
		}
		delete messageData;
		return nullptr;
	}
	size_t NetMessageProxy::WriteToBuffer(char * buffer, const size_t size)
	{
		if (this->mMsgType < RequestEnd)
		{
			return 0;
		}
		size_t offset = sizeof(unsigned int);
		size_t size = sizeof(this->mActionId);
		memcpy(buffer, &this->mActionId, sizeof(this->mActionId));
		offset += sizeof(this->mActionId);
		if (this->mRpcId != 0)
		{		
			memcpy(buffer, &this->mRpcId, sizeof(this->mRpcId));
			offset += sizeof(this->mRpcId);
		}
		if (this->mUserId != 0)
		{		
			memcpy(buffer, &this->mUserId, sizeof(this->mUserId));
			offset += sizeof(this->mUserId);
		}
		if (this->mReqMessage != nullptr)
		{		
			if (!this->mReqMessage->SerializePartialToArray(buffer + offset, size - offset))
			{
				return false;
			}
			offset += this->mReqMessage->ByteSizeLong();
		}
		if (!this->mJsonString.empty())
		{
			memcpy(buffer + offset, this->mJsonString.c_str(), this->mJsonString.size());
			offset += this->mJsonString.size();
		}
		memcpy(buffer, &offset, sizeof(unsigned int));
		return offset;
	}
	bool NetMessageProxy::SetCode(XCode code)
	{
		if (this->mRpcId == 0)
		{
			return false;
		}
		if (this->mMsgType >= RequestEnd)
		{
			return false;
		}
		if (this->mMsgType == s2sRequest)
		{
			this->mCode = code;
			this->mMsgType = s2sResponse;
			return true;
		}
		if (this->mMsgType == c2sRequest)
		{
			this->mCode = code;
			this->mMsgType = c2sResponse;
			return true;
		}
		return false;
	}
}


