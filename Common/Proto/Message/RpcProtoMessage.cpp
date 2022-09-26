//
// Created by yjz on 2022/5/18.
//

#include"RpcProtoMessage.h"
#include"Client/Rpc.h"
namespace Tcp
{
	namespace Rpc
	{
		RpcProtoMessage::RpcProtoMessage()
		{
            this->mPorto = Tcp::Porto::Protobuf;
		}

		int RpcProtoMessage::Serailize(std::ostream& os)
        {
            this->Write(os, this->mMessage->ByteSize());
            this->Write(os, (char) this->mType);
            this->Write(os, (char) this->mPorto);
            this->mMessage->SerializePartialToOstream(&os);
            return 0;
        }
	}
}