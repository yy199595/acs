
#include <Core/App.h>
#include <Scene/LuaScriptComponent.h>
#include <Timer/TimerComponent.h>
#include <Coroutine/TaskComponent.h>
#include <Listener/TcpServerComponent.h>
#include <Scene/MysqlComponent.h>
#include <Scene/RpcConfigComponent.h>
#include <Scene/RedisComponent.h>
#include <Scene/MysqlProxyComponent.h>
#include <Component/Http/HttpComponent.h>
#include"../Gate/Service/GateService.h"
#include <Telnet/TelnetClientComponent.h>

#include <Service/LuaServiceMgrComponent.h>
#include <ServerRpc/ProtoRpcComponent.h>
#include <Service/NodeProxyComponent.h>
#include <Scene/ThreadPoolComponent.h>
#include <ServerRpc/ProtoRpcClientComponent.h>
#include <Service/AccountService.h>
#include <Service/LocalHostService.h>
#include <Service/CenterHostService.h>
#include <Scene/MonitorComponent.h>
#include <Service/MysqlService.h>
#include <Http/HttpResourceComponent.h>
#include <Service/HttpLoginService.h>
#include <Http/HttpOperComponent.h>
#include "Scene/OperatorComponent.h"
#include"Scene/LoggerComponent.h"
#include"Component/ProtoGateComponent.h"
#include"Component/ProtoGateClientComponent.h"
using namespace GameKeeper;

int main(int argc, char **argv)
{
    REGISTER_COMPONENT(TimerComponent);
    REGISTER_COMPONENT(ProtoRpcComponent);
    REGISTER_COMPONENT(RedisComponent);
    REGISTER_COMPONENT(MysqlComponent);
    REGISTER_COMPONENT(LuaScriptComponent);
    REGISTER_COMPONENT(RpcConfigComponent);
    REGISTER_COMPONENT(MysqlProxyComponent);
    REGISTER_COMPONENT(HttpComponent);
    REGISTER_COMPONENT(OperatorComponent);
	REGISTER_COMPONENT(LoggerComponent);

    REGISTER_COMPONENT(ThreadPoolComponent);
    REGISTER_COMPONENT(TaskComponent);
    REGISTER_COMPONENT(NodeProxyComponent);
    REGISTER_COMPONENT(TcpServerComponent);
    REGISTER_COMPONENT(ProtoRpcClientComponent);
    REGISTER_COMPONENT(TelnetClientComponent);
    REGISTER_COMPONENT(MonitorComponent);

    REGISTER_COMPONENT(LuaServiceMgrComponent);
    REGISTER_COMPONENT(ProtoGateComponent);
    REGISTER_COMPONENT(ProtoGateClientComponent);

    REGISTER_COMPONENT(MysqlService);
    REGISTER_COMPONENT(GateService);
    REGISTER_COMPONENT(AccountService);
    REGISTER_COMPONENT(CenterHostService);
    REGISTER_COMPONENT(LocalHostService);
    REGISTER_COMPONENT(HttpLoginService);
    REGISTER_COMPONENT(HttpOperComponent);
    REGISTER_COMPONENT(HttpResourceComponent);

	if (argc == 1)
	{
		argv[1] = new char[100];
		argv[1] = "./Config/server.json";
	}


    App app;
    return app.Run(argc, argv);
}