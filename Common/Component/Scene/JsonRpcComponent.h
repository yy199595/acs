//
// Created by mac on 2021/11/20.
//

#ifndef GAMEKEEPER_JSONRPCCOMPONENT_H
#define GAMEKEEPER_JSONRPCCOMPONENT_H
#include"Component.h"
#include"Util/JsonHelper.h"
#include"Network/Rpc/JsonRpcClient.h"
namespace GameKeeper
{
    class JsonRpcComponent : public Component, public ISocketListen,
                             public IRpc<RapidJsonReader, RapidJsonReader>
    {
    public:
        JsonRpcComponent() = default;
        ~JsonRpcComponent() final = default;

    public:
        JsonRpcClient * GetSession(long long id);

    public:
        void StartClose(long long id) final;
        void OnConnectAfter(long long id, XCode code) final;
        void OnCloseSocket(long long id, XCode code) final;
        void OnRequest(long long id, RapidJsonReader *t1) final;
        void OnResponse(long long id, RapidJsonReader *t2) final;
    protected:
        bool Awake() override;
        void OnListen(SocketProxy *socket) override;


    private:
        class RpcConfigComponent * mRpcConfigComponent;
        class RpcRequestComponent * mRequestComponent;
        class RpcResponseComponent * mResponseComponent;
        std::unordered_map<long long, JsonRpcClient *> mClientMap;
    };
}
#endif //GAMEKEEPER_JSONRPCCOMPONENT_H
