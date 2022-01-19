//
// Created by mac on 2021/11/28.
//

#include"RpcProxyClient.h"
#include"Protocol/c2s.pb.h"
#include"Core/App.h"
#ifdef __DEBUG__
#include"google/protobuf/util/json_util.h"
#endif
#include"Component/GateClientComponent.h"
namespace GameKeeper
{
    RpcProxyClient::RpcProxyClient(std::shared_ptr<SocketProxy> socket, SocketType type,
                                   GateClientComponent *component)
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
    
    XCode RpcProxyClient::OnResponse(const char *buffer, size_t size) //不处理response消息
    {
        return XCode::UnKnowPacket;
    }

    void RpcProxyClient::OnClientError(XCode code)
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

    void RpcProxyClient::StartClose()
    {
        XCode code = XCode::NetActiveShutdown;
        this->mNetWorkThread.Invoke(&RpcProxyClient::OnClientError, this, code);
    }


    bool RpcProxyClient::SendToClient(std::shared_ptr<c2s::Rpc_Request> message)
    {
        if(!this->IsOpen()) return false;
        if(this->mNetWorkThread.IsCurrentThread())
        {
            this->SendData(RPC_TYPE_REQUEST, message);
            return true;
        }
        this->mNetWorkThread.Invoke(&RpcProxyClient::SendData, this, RPC_TYPE_REQUEST, message);
        return true;
    }

    bool RpcProxyClient::SendToClient(std::shared_ptr<c2s::Rpc_Response> message)
    {
        if(!this->IsOpen()) return false;
        if(this->mNetWorkThread.IsCurrentThread())
        {
            this->SendData(RPC_TYPE_RESPONSE, message);
            return true;
        }
        this->mNetWorkThread.Invoke(&RpcProxyClient::SendData, this, RPC_TYPE_RESPONSE, message);
        return true;
    }

    void RpcProxyClient::OnSendData(XCode code, std::shared_ptr<Message> message)
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