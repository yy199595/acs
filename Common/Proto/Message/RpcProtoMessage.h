//
// Created by yjz on 2022/5/18.
//

#ifndef _RPCPROTOMESSAGE_H_
#define _RPCPROTOMESSAGE_H_
#include"ProtoMessage.h"
#include"Client/Rpc.h"
using namespace google::protobuf;
namespace Tcp
{
    namespace Rpc
    {
        class RpcProtoMessage final : public Tcp::ProtoMessage
        {
        public:
            RpcProtoMessage();

            int Serailize(std::ostream &os) final;

        public:
            Tcp::Type mType;
            Tcp::Porto mPorto;
            std::shared_ptr<Message> mMessage;
        };
    }
}

#endif //_RPCPROTOMESSAGE_H_
