//
// Created by mac on 2021/11/28.
//

#include"RpcGateClient.h"
#include"Protocol/c2s.pb.h"
#include"Object/App.h"
#ifdef __DEBUG__
#include"google/protobuf/util/json_util.h"
#endif
#include"Component/GateClientComponent.h"
namespace Sentry
{
    RpcGateClient::RpcGateClient(std::shared_ptr<SocketProxy> socket, SocketType type,
                                 GateClientComponent *component)
        : RpcClient(socket, type), mProxyComponent(component)
    {
        this->mQps = 0;
        this->mCallCount = 0;
    }

    void RpcGateClient::OnConnect(XCode code)
    {

    }

    XCode RpcGateClient::OnRequest(const char *buffer, size_t size)
    {
        std::shared_ptr<c2s::Rpc_Request> request(new c2s::Rpc_Request());
        if (!request->ParseFromArray(buffer, (int)size))
        {
            return XCode::ParseRequestDataError;
        }
        this->mCallCount++;
        this->mQps += size;
        std::cout << "receive player message count = " << this->mCallCount << std::endl;
        request->set_sock_id(this->GetSocketId());
        MainTaskScheduler &mainTaskScheduler = App::Get().GetTaskScheduler();
        mainTaskScheduler.Invoke(&GateClientComponent::OnRequest, this->mProxyComponent, request);
        return XCode::Successful;
    }
    
    XCode RpcGateClient::OnResponse(const char *buffer, size_t size) //不处理response消息
    {
        return XCode::UnKnowPacket;
    }

    void RpcGateClient::OnClientError(XCode code)
    {
        if(code == XCode::NetActiveShutdown)
        {
            this->mSocketProxy->Close();
            return;
        }
        long long id = this->GetSocketId();
        MainTaskScheduler &mainTaskScheduler = App::Get().GetTaskScheduler();
        mainTaskScheduler.Invoke(&GateClientComponent::OnCloseSocket, this->mProxyComponent, id, code);
    }

    void RpcGateClient::StartClose()
    {
        XCode code = XCode::NetActiveShutdown;
        this->mNetWorkThread.Invoke(&RpcGateClient::OnClientError, this, code);
    }


    bool RpcGateClient::SendToClient(std::shared_ptr<c2s::Rpc_Request> message)
    {
        if(!this->IsOpen()) return false;
        if(this->mNetWorkThread.IsCurrentThread())
        {
            this->SendData(RPC_TYPE_REQUEST, message);
            return true;
        }
        this->mNetWorkThread.Invoke(&RpcGateClient::SendData, this, RPC_TYPE_REQUEST, message);
        return true;
    }

    bool RpcGateClient::SendToClient(std::shared_ptr<c2s::Rpc_Response> message)
    {
        if(!this->IsOpen()) return false;
        if(this->mNetWorkThread.IsCurrentThread())
        {
            this->SendData(RPC_TYPE_RESPONSE, message);
            return true;
        }
        this->mNetWorkThread.Invoke(&RpcGateClient::SendData, this, RPC_TYPE_RESPONSE, message);
        return true;
    }

    void RpcGateClient::OnSendData(XCode code, std::shared_ptr<Message> message)
    {
#ifdef __DEBUG__
        if(code != XCode::Successful)
        {
            std::string json;
            util::MessageToJsonString(*message, &json);
            std::cout << "send message to client error : " << json << std::endl;
        }
#endif
    }
}