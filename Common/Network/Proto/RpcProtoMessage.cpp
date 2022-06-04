//
// Created by yjz on 2022/5/18.
//

#include"RpcProtoMessage.h"
#include"Network/Rpc.h"
namespace Tcp
{
	namespace Rpc
	{
		RpcProtoMessage::RpcProtoMessage(MESSAGE_TYPE type, std::shared_ptr<Message> message)
				:mType(type), mMessage(message)
		{

		}

		int RpcProtoMessage::Serailize(std::ostream& os)
		{
			this->Write(os, this->mMessage->ByteSize() + 1);
			this->Write(os, (char)this->mType);
			this->mMessage->SerializePartialToOstream(&os);
			return 0;
		}
	}
}