//
// Created by zmhy0073 on 2021/12/1.
//

#ifndef GAMEKEEPER_GATESERVICE_H
#define GAMEKEEPER_GATESERVICE_H
#include"Component/ServiceBase/ServiceComponentBase.h"
namespace GameKeeper
{
    class GateService : public ServiceComponentBase
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
    private:
        class GateClientComponent * mGateComponent;
    };

}

#endif //GAMEKEEPER_GATESERVICE_H
