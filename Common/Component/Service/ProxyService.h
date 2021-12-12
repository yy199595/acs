//
// Created by zmhy0073 on 2021/12/1.
//

#ifndef GAMEKEEPER_PROXYSERVICE_H
#define GAMEKEEPER_PROXYSERVICE_H
#include"ProtoServiceComponent.h"
namespace GameKeeper
{
    class ProxyService : public ProtoServiceComponent, public INodeProxyRefresh
    {
    public:
        ProxyService();
        ~ProxyService() final = default;
    protected:
        bool Awake() final;
        bool LateAwake() final;
        void OnAddProxyNode(class RpcNodeProxy *node) final;
        void OnDelProxyNode(class RpcNodeProxy *node) final;
    private:
        XCode Ping();
        XCode Login(const c2s::ProxyLogin_Request & request);
    private:
        class ProtoProxyClientComponent * mProxyComponent;
    };

}

#endif //GAMEKEEPER_PROXYSERVICE_H
