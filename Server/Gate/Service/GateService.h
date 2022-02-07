//
// Created by zmhy0073 on 2021/12/1.
//

#ifndef GAMEKEEPER_GATESERVICE_H
#define GAMEKEEPER_GATESERVICE_H
#include"Service/RpcService.h"
namespace Sentry
{
    class GateService : public RpcService
    {
    public:
        GateService() = default;
        ~GateService() final = default;
    protected:
        bool Awake() final;
        bool LateAwake() final;
    private:
        XCode Ping();
        XCode Login(const c2s::ProxyLogin_Request & request);
        XCode Allot(const s2s::AddToGate_Request & request, s2s::AddToGate_Response & response);
    private:
        class GateClientComponent * mGateComponent;
    };

}

#endif //GAMEKEEPER_GATESERVICE_H
