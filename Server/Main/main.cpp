
#include <Core/App.h>
#include <Scene/LuaScriptComponent.h>
#include <Timer/TimerComponent.h>
#include <Coroutine/TaskComponent.h>
#include <Listener/TcpServerComponent.h>
#include "Component/MysqlComponent.h"
#include <Scene/RpcConfigComponent.h>
#include "Component/RedisComponent.h"
#include "Component/MysqlProxyComponent.h"
#include "Http/Component/HttpClientComponent.h"
#include"../Gate/Service/GateService.h"
#include <Telnet/TelnetClientComponent.h>

#include <Service/LuaServiceMgrComponent.h>
#include <Rpc/RpcComponent.h>
#include <Service/RpcNodeComponent.h>
#include <Scene/ThreadPoolComponent.h>
#include <Rpc/RpcClientComponent.h>
#include <Service/AccountService.h>
#include <Service/LocalHostService.h>
#include <Service/CenterHostService.h>
#include <Scene/MonitorComponent.h>
#include "Service/MysqlService.h"
#include <Service/HttpLoginService.h>
#include "Http/Service/HttpOperComponent.h"
#include "Scene/OperatorComponent.h"
#include"Scene/LoggerComponent.h"
#include"Component/GateComponent.h"
#include"Component/GateClientComponent.h"
using namespace GameKeeper;

int main(int argc, char **argv)
{
    REGISTER_COMPONENT(TimerComponent);
    REGISTER_COMPONENT(RpcComponent);
    REGISTER_COMPONENT(RedisComponent);
    REGISTER_COMPONENT(MysqlComponent);
    REGISTER_COMPONENT(LuaScriptComponent);
    REGISTER_COMPONENT(RpcConfigComponent);
    REGISTER_COMPONENT(MysqlProxyComponent);
    REGISTER_COMPONENT(HttpClientComponent);
    REGISTER_COMPONENT(OperatorComponent);
    REGISTER_COMPONENT(LoggerComponent);

    REGISTER_COMPONENT(ThreadPoolComponent);
    REGISTER_COMPONENT(TaskComponent);
    REGISTER_COMPONENT(RpcNodeComponent);
    REGISTER_COMPONENT(TcpServerComponent);
    REGISTER_COMPONENT(RpcClientComponent);
    REGISTER_COMPONENT(TelnetClientComponent);
    REGISTER_COMPONENT(MonitorComponent);

    REGISTER_COMPONENT(LuaServiceMgrComponent);
    REGISTER_COMPONENT(GateComponent);
    REGISTER_COMPONENT(GateClientComponent);

    REGISTER_COMPONENT(MysqlService);
    REGISTER_COMPONENT(GateService);
    REGISTER_COMPONENT(AccountService);
    REGISTER_COMPONENT(CenterHostService);
    REGISTER_COMPONENT(LocalHostService);
    REGISTER_COMPONENT(HttpLoginService);
    REGISTER_COMPONENT(HttpOperComponent);

    if (argc == 1) {
        argv[1] = new char[100];
        argv[1] = "./Config/server.json";
    }


    App app;
    return app.Run(argc, argv);
}