//
// Created by zmhy0073 on 2022/10/25.
//

#include"Helper.h"
#include"Config/ServiceConfig.h"
namespace Sentry
{
    std::shared_ptr<Rpc::Packet> Helper::MakeRpcPacket(const std::string &fullName)
    {
        if(RpcConfig::Inst()->GetMethodConfig(fullName) == nullptr)
        {
            return nullptr;
        }
        std::shared_ptr<Rpc::Packet> message = std::make_shared<Rpc::Packet>();
        {
            message->GetHead().Add("func", fullName);
        }
        return std::move(message);
    }

    std::shared_ptr<Rpc::Packet> Helper::MakeRpcPacket(const std::string &service, const std::string &func)
    {
        const RpcServiceConfig * rpcServiceConfig = RpcConfig::Inst()->GetConfig(service);
        if(rpcServiceConfig == nullptr)
        {
            return nullptr;
        }
        const RpcMethodConfig * methodConfig = rpcServiceConfig->GetMethodConfig(func);
        if(methodConfig == nullptr)
        {
            return nullptr;
        }
        std::shared_ptr<Rpc::Packet> message = std::make_shared<Rpc::Packet>();
        {
            message->GetHead().Add("func", methodConfig->FullName);
        }
        return std::move(message);
    }
}