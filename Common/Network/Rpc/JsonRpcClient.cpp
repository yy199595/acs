//
// Created by mac on 2021/11/20.
//
#include"Core/App.h"
#include"JsonRpcClient.h"
#include"Define/CommonTypeDef.h"
#include"Scene/JsonRpcComponent.h"
namespace GameKeeper
{
    JsonRpcClient::JsonRpcClient(JsonRpcComponent *component, SocketProxy *socket, SocketType type)
        : RpcClient(socket, type), mRpcComponent(component)
    {

    }

    bool JsonRpcClient::OnRequest(const char *buffer, size_t size)
    {
        auto jsonReader = new RapidJsonReader();
        if (!jsonReader->TryParse(buffer, size))
        {
            delete jsonReader;
            return false;
        }
        long long id = this->mSocketProxy->GetSocketId();
        MainTaskScheduler &taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.Invoke(&JsonRpcComponent::OnRequest, this->mRpcComponent, id, jsonReader);
        return true;
    }

    void JsonRpcClient::OnConnect(XCode code)
    {
        long long id = this->mSocketProxy->GetSocketId();
        MainTaskScheduler &taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.Invoke(&JsonRpcComponent::OnConnectAfter, this->mRpcComponent, id, code);
    }

    bool JsonRpcClient::OnResponse(const char *buffer, size_t size)
    {
        auto jsonReader = new RapidJsonReader();
        if (!jsonReader->TryParse(buffer, size))
        {
            delete jsonReader;
            return false;
        }
        long long id = this->mSocketProxy->GetSocketId();
        MainTaskScheduler &taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.Invoke(&JsonRpcComponent::OnResponse, this->mRpcComponent, id, jsonReader);
        return true;
    }

    void JsonRpcClient::CloseSocket(XCode code)
    {
        this->mSocketProxy->Close();
        MainTaskScheduler &taskScheduler = App::Get().GetTaskScheduler();
    }

    bool JsonRpcClient::StartSendJson(char type, const RapidJsonWriter *jsonData)
    {
        if(!this->IsOpen())
        {
            return false;
        }
        if(mNetWorkThread.IsCurrentThread())
        {
           this->StartSendJson(type, jsonData);
            return true;
        }
        mNetWorkThread.Invoke(&JsonRpcClient::SendJsonData, this, type, jsonData);
        return true;
    }

    void JsonRpcClient::SendJsonData(char type, const RapidJsonWriter *jsonData)
    {
        size_t size = 0;
        LocalObject<RapidJsonWriter> lock(jsonData);
        const char *json = jsonData->GetData(size);
        AsioTcpSocket &socket = this->mSocketProxy->GetSocket();
        char *sendBuffer = new char[sizeof(char) + sizeof(TCP_HEAD) + size];

        sendBuffer[0] = type;
        memcpy(sendBuffer + 1, &size, sizeof(TCP_HEAD));
        memcpy(sendBuffer + 1 + sizeof(TCP_HEAD), json, size);
        this->AsyncSendMessage(sendBuffer, 1 + sizeof(unsigned int) + size);
    }

    void JsonRpcClient::OnSendAfter(XCode code, const char *buffer, size_t size)
    {
        delete []buffer;
    }
}