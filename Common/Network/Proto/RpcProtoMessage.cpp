//
// Created by yjz on 2022/5/18.
//

#include"RpcProtoMessage.h"
#include"Network/Rpc.h"
namespace Tcp
{
	namespace Rpc
	{
		RpcProtoMessage::RpcProtoMessage()
		{
            this->mPorto = MESSAGE_PROTO::MSG_RPC_PROTOBUF;
		}

		int RpcProtoMessage::Serailize(std::ostream& os)
        {
            this->Write(os, this->mMessage->ByteSize() + 2);
            this->Write(os, (char) this->mType);
            this->Write(os, (char) this->mPorto);
            this->mMessage->SerializePartialToOstream(&os);
            return 0;
        }
	}
}