#include "PacketMapper.h"

#include <Core/App.h>
#include <Util/JsonHelper.h>
#include <Scene/ProtocolComponent.h>
namespace Sentry
{
    template<typename T>
    bool MemoryCopy(T *tar, const char *sou, size_t &offset, const size_t maxsize)
    {
        memcpy(tar, sou + offset, sizeof(T));
        offset += sizeof(T);
        return offset <= maxsize;
    }


    PacketMapper::PacketMapper(NetMessageType type)
            : mMsgType(type)
    {
        this->Clear();
    }

    PacketMapper::~PacketMapper()
    {
        this->Clear();
    }

    void PacketMapper::Clear()
    {
        this->mRpcId = 0;
        this->mUserId = 0;
		this->mAddress.clear();
		this->mMessageData.clear();
		this->mProConfig = nullptr;
    }

	thread_local std::queue<PacketMapper *> PacketMapper::mPacketPool;

	PacketMapper *PacketMapper::Create(const std::string & address, const char *message, const size_t size)
    {
        if (message == nullptr || size == 0)
        {
            return nullptr;
        }
        size_t offset = 1;
        ProtocolComponent *pProtocolMgr = Scene::GetComponent<ProtocolComponent>();

        int code = 0;
		long long userid = 0;
        unsigned int rpcid = 0;
        unsigned short actionid = 0;     
        NetMessageType messageType = (NetMessageType) message[0];
        SayNoAssertRetNull_F(MemoryCopy(&actionid, message, offset, size));
		const ProtocolConfig * config = pProtocolMgr->GetProtocolConfig(actionid);
        SayNoAssertRetNull_F(config);

        switch (messageType)
        {
            case S2S_REQUEST:
				SayNoAssertRetNull_F(MemoryCopy(&rpcid, message, offset, size));
                break;
            case S2S_RESPONSE:
				SayNoAssertRetNull_F(MemoryCopy(&code, message, offset, size));
                SayNoAssertRetNull_F(MemoryCopy(&rpcid, message, offset, size));
                break;
            case S2S_NOTICE:

                break;
            case C2S_REQUEST:
				SayNoAssertRetNull_F(MemoryCopy(&rpcid, message, offset, size));
                SayNoAssertRetNull_F(MemoryCopy(&userid, message, offset, size));
                break;
            case C2S_RESPONSE:
				SayNoAssertRetNull_F(MemoryCopy(&rpcid, message, offset, size));
                SayNoAssertRetNull_F(MemoryCopy(&userid, message, offset, size));
                SayNoAssertRetNull_F(MemoryCopy(&code, message, offset, size));
                break;
            case C2S_NOTICE:
            case S2C_REQUEST:
				SayNoAssertRetNull_F(MemoryCopy(&userid, message, offset, size));
            default:
                return nullptr;
        }

		PacketMapper *messageData = PacketMapper::Create(messageType);
	
        messageData->mRpcId = rpcid;
        messageData->mUserId = userid;
        messageData->mCode = (XCode) code;
        messageData->mProConfig = config;
		messageData->mAddress = address;
        if (offset < size)
        {
            const char *msg = message + offset;
            messageData->mMessageData.append(msg, size - offset);
        }
        return messageData;
    }

    PacketMapper *PacketMapper::Create(const std::string & address, NetMessageType type, const std::string &service, const std::string &method)
    {
        if (type > REQUEST_END)
        {
            return nullptr;
        }
        ProtocolComponent *pProtocolMgr = Scene::GetComponent<ProtocolComponent>();

        const ProtocolConfig *config = pProtocolMgr->GetProtocolConfig(service, method);
        if (config != nullptr)
        {
			PacketMapper * messageData = PacketMapper::Create(type);
			messageData->mAddress = address;
            messageData->mProConfig = config;
            return messageData;
        }
        return nullptr;
    }

	bool PacketMapper::SetCode(XCode code)
	{
		if (this->mRpcId == 0)
		{
			return false;
		}
		this->mCode = code;
		switch (this->mMsgType)
		{
		case NetMessageType::C2S_REQUEST:
			this->mMsgType = S2S_RESPONSE;
			return this->mUserId != 0;
		case NetMessageType::S2S_REQUEST:
			this->mMsgType = S2S_RESPONSE;
			return true;
		}
		return false;
	}

	void PacketMapper::SetType(NetMessageType type)
	{
		this->mMsgType = type;
		this->mMessageData.clear();
	}

	bool PacketMapper::SetRpcId(const unsigned int id)
	{
		if (id == 0)return false;
		this->mRpcId = id;
		return true;
	}

	bool PacketMapper::SetMessage(const Message * message)
	{
		if (message == nullptr)
		{
			return false;
		}
		return message->SerializePartialToString(&mMessageData);
	}

	bool PacketMapper::SetMessage(const Message & message)
	{
		return message.SerializePartialToString(&mMessageData);
	}

	bool PacketMapper::SetMessage(const char * message, const size_t size)
	{
		if (message == nullptr || size == 0)
		{
			return false;
		}
		this->mMessageData.clear();
		this->mMessageData.append(message, size);
		return true;
	}

	size_t PacketMapper::GetPackageSize()
	{
		size_t size = sizeof(char);
		size += sizeof(this->mProConfig->MethodId);
		size += this->mRpcId == 0 ? 0 : sizeof(this->mRpcId);
		size += this->mUserId == 0 ? 0 : sizeof(this->mUserId);
		size += this->mMessageData.size();
		return size;
	}

	PacketMapper * PacketMapper::Create(NetMessageType type)
	{
		PacketMapper * packet = nullptr;
		if (!mPacketPool.empty())
		{
			packet = mPacketPool.front();
			packet->mMsgType = type;
			mPacketPool.pop();
			return packet;
		}
		return new PacketMapper(type);
	}


    size_t PacketMapper::WriteToBuffer(char *buffer, const size_t size)
    {      
		size_t offset = sizeof(unsigned int);
		unsigned char type = (unsigned char)this->mMsgType;  
		memcpy(buffer + offset, &type, sizeof(type)); offset += sizeof(type);
		memcpy(buffer + offset, &mProConfig->MethodId, sizeof(this->mProConfig->MethodId));

		offset += sizeof(this->mProConfig->MethodId);

		if (this->mMsgType > REQUEST_END)
		{
			int code = (int)this->mCode;
			memcpy(buffer + offset, &code, sizeof(code)); offset += sizeof(code);
		}

        if (this->mRpcId != 0)
        {          	
			memcpy(buffer + offset, &mRpcId, sizeof(mRpcId)); offset += sizeof(mRpcId);
        }
		
        if (this->mUserId != 0)
        {
			memcpy(buffer + offset, &mUserId, sizeof(mUserId)); offset += sizeof(mUserId);
        }	
        memcpy(buffer + offset, this->mMessageData.c_str(), this->mMessageData.size());
        offset += this->mMessageData.size();
		size_t sumSize = offset - sizeof(unsigned int);
        memcpy(buffer, &sumSize, sizeof(unsigned int));
        return offset;
    }
	void PacketMapper::Destory()
	{
		if (this->mPacketPool.size() >= 100)
		{
			delete this;
			return;
		}
		else
		{			
			this->mRpcId = 0;
			this->mUserId = 0;
			this->mAddress.clear();
			this->mMsgType = S2S_NONE;
			this->mProConfig = nullptr;
			this->mMessageData.clear();
			mPacketPool.push(this);
		}	
	}
}
