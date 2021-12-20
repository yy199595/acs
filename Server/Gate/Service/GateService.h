//
// Created by zmhy0073 on 2021/12/1.
//

#ifndef GAMEKEEPER_GATESERVICE_H
#define GAMEKEEPER_GATESERVICE_H
#include"Component/Service/ServiceComponent.h"
namespace GameKeeper
{
    class GateService : public ServiceComponent, public INodeRefresh
    {
    public:
        GateService() = default;
        ~GateService() final = default;
    protected:
        bool Awake() final;
        bool LateAwake() final;
        void OnAddRpcNode(class RpcNode *node) final;
        void OnDelRpcNode(class RpcNode *node) final;
    private:
        XCode Ping();
        XCode Login(const c2s::ProxyLogin_Request & request);
    private:
        class GateClientComponent * mGateComponent;
    };

}

#endif //GAMEKEEPER_GATESERVICE_H
