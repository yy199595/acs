//
// Created by mac on 2021/11/20.
//

#ifndef GAMEKEEPER_JSONRPCCLIENT_H
#define GAMEKEEPER_JSONRPCCLIENT_H
#include<stack>
#include"RpcClient.h"
namespace GameKeeper
{
    class JsonRpcComponent;
    class JsonRpcClient : public RpcClient
    {
    public:
         JsonRpcClient(JsonRpcComponent * component, SocketProxy * socket, SocketType type);
    public:
        bool StartSendJson(char type, const RapidJsonWriter * jsonData);
    protected:
        void OnConnect(XCode code) final;
        void CloseSocket(XCode code) final;
        bool OnRequest(const char *buffer, size_t size) final;
        bool OnResponse(const char *buffer, size_t size) final;
        void OnSendAfter(XCode code, const char *buffer, size_t size) final;
    private:
        void SendJsonData(char type, const RapidJsonWriter * jsonData);     
    private:
        std::stack<char> mJsonRecords;
		JsonRpcComponent * mRpcComponent;
    };
}
#endif //GAMEKEEPER_JSONRPCCLIENT_H
