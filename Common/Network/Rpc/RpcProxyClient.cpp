//
// Created by mac on 2021/11/28.
//

#include"RpcProxyClient.h"
#include"Protocol/c2s.pb.h"
#include"Core/App.h"
#ifdef __DEBUG__
#include<google/protobuf/util/json_util.h>
#endif
#include"Component/ProtoGateClientComponent.h"
namespace GameKeeper
{
    RpcProxyClient::RpcProxyClient(SocketProxy * socket, SocketType type,
                                   ProtoGateClientComponent *component)
        : RpcClient(socket, type), mProxyComponent(component)
    {
        this->mQps = 0;
        this->mCallCount = 0;
    }

    void RpcProxyClient::OnConnect(XCode code)
    {

    }

    XCode RpcProxyClient::OnRequest(const char *buffer, size_t size)
    {
        auto request = new c2s::Rpc_Request();
        if (!request->ParseFromArray(buffer, (int)size))
        {
            delete request;
            return XCode::ParseRequestDataError;
        }
        this->mCallCount++;
        this->mQps += size;
        std::cout << "receive player message count = " << this->mCallCount << std::endl;
        request->set_sockid(this->GetSocketId());
        MainTaskScheduler &mainTaskScheduler = App::Get().GetTaskScheduler();
        mainTaskScheduler.Invoke(&ProtoGateClientComponent::OnRequest, this->mProxyComponent, request);
        return XCode::Successful;
    }
    
    XCode RpcProxyClient::OnResponse(const char *buffer, size_t size) //不处理response消息
    {
        return XCode::UnKnowPacket;
    }

    void RpcProxyClient::OnClose(XCode code)
    {
        long long id = this->GetSocketId();
        MainTaskScheduler &mainTaskScheduler = App::Get().GetTaskScheduler();
        mainTaskScheduler.Invoke(&ProtoGateClientComponent::OnCloseSocket, this->mProxyComponent, id, code);
    }

    void RpcProxyClient::StartClose()
    {
        XCode code = XCode::NetActiveShutdown;
        if(this->mNetWorkThread.IsCurrentThread())
        {
            this->CloseSocket(code);
        }
        this->mNetWorkThread.Invoke(&RpcProxyClient::CloseSocket, this, code);
    }


    bool RpcProxyClient::SendToClient(const c2s::Rpc_Request *message)
    {
        if(!this->IsOpen())
        {
            return false;
        }
        if(this->mNetWorkThread.IsCurrentThread())
        {
            this->SendData(RPC_TYPE_REQUEST, message);
            return true;
        }
        this->mNetWorkThread.Invoke(&RpcProxyClient::SendData, this, RPC_TYPE_REQUEST, message);
        return true;
    }

    bool RpcProxyClient::SendToClient(const c2s::Rpc_Response *message)
    {
        if(!this->IsOpen())
        {
            return false;
        }
        if(this->mNetWorkThread.IsCurrentThread())
        {
            this->SendData(RPC_TYPE_RESPONSE, message);
            return true;
        }
        this->mNetWorkThread.Invoke(&RpcProxyClient::SendData, this, RPC_TYPE_RESPONSE, message);
        return true;
    }

    void RpcProxyClient::OnSendData(XCode code, const Message * message)
    {
#ifdef __DEBUG__
        if(code != XCode::Successful)
        {
            std::string json;
            util::MessageToJsonString(*message, &json);
            std::cout << "send message to client error : " << json << std::endl;
        }
#endif
        delete message;
    }
}