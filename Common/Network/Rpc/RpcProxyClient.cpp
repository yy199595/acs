//
// Created by mac on 2021/11/28.
//

#include"RpcProxyClient.h"
#include"Protocol/c2s.pb.h"
#include"Core/App.h"
#ifdef __DEBUG__
#include<google/protobuf/util/json_util.h>
#endif
#include"ProxyRpc/ProtoProxyClientComponent.h"
namespace GameKeeper
{
    RpcProxyClient::RpcProxyClient(SocketProxy * socket, SocketType type,
                                   ProtoProxyClientComponent *component)
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
        mainTaskScheduler.Invoke(&ProtoProxyClientComponent::OnRequest, this->mProxyComponent, request);
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
        mainTaskScheduler.Invoke(&ProtoProxyClientComponent::OnCloseSocket, this->mProxyComponent, id, code);
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

    bool RpcProxyClient::StartSendData(char type, const Message *message)
    {
        if(!this->IsOpen())
        {
            return false;
        }
        if(this->mNetWorkThread.IsCurrentThread())
        {
            this->SendData(type, message);
            return true;
        }
        this->mNetWorkThread.Invoke(&RpcProxyClient::SendData, this, type, message);
        return true;
    }

    void RpcProxyClient::SendData(char type, const Message *message)
    {
        LocalObject<Message> lock(message);
        const int body = message->ByteSize();
        size_t head = sizeof(char) + sizeof(int);
		
		char * buffer = new char[head + body];

        buffer[0] = type;
        memcpy(buffer + sizeof(char), &body, sizeof(body));
        if (!message->SerializePartialToArray(buffer + head, body))
        {
            return;
        }
        this->AsyncSendMessage(std::move(buffer), head + body);
    }
}