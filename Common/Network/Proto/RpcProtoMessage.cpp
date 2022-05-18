//
// Created by yjz on 2022/5/18.
//

#include "RpcProtoMessage.h"
#include"Network/Rpc.h"

namespace Rpc
{
	RpcRequestMessage::RpcRequestMessage(std::shared_ptr<com::Rpc::Request> message)
		: mMessage(message)
	{

	}

	bool RpcRequestMessage::Serailize(std::ostream& os)
	{
		this->Write(os, this->mMessage->ByteSize());
		this->Write(os, (char)RPC_TYPE_REQUEST);
		return this->mMessage->SerializePartialToOstream(&os);
	}
}

namespace Rpc
{
	RpcResponseMessage::RpcResponseMessage(std::shared_ptr<com::Rpc::Response> message)
		: mMessage(message)
	{

	}

	bool RpcResponseMessage::Serailize(std::ostream& os)
	{
		this->Write(os, this->mMessage->ByteSize());
		this->Write(os, (char)RPC_TYPE_REQUEST);
		return this->mMessage->SerializePartialToOstream(&os);
	}
}