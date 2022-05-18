//
// Created by yjz on 2022/5/18.
//

#ifndef _RPCPROTOMESSAGE_H_
#define _RPCPROTOMESSAGE_H_
#include"ProtoMessage.h"

namespace Rpc
{
	class RpcRequestMessage final : public Tcp::ProtoMessage
	{
	 public:
		RpcRequestMessage(std::shared_ptr<com::Rpc::Request> message);
	 protected:
		bool Serailize(std::ostream& os) final;
	 private:
		std::shared_ptr<com::Rpc::Request> mMessage;
	};

	class RpcResponseMessage final : public Tcp::ProtoMessage
	{
	 public:
		RpcResponseMessage(std::shared_ptr<com::Rpc::Response> message);
	 protected:
		bool Serailize(std::ostream& os) final;
	 private:
		std::shared_ptr<com::Rpc::Response> mMessage;
	};
}

#endif //_RPCPROTOMESSAGE_H_
