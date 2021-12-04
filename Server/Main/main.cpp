
#include <Core/App.h>
#include <Scene/LuaScriptComponent.h>
#include <Timer/TimerComponent.h>
#include <Coroutine/CoroutineComponent.h>
#include <Listener/TcpServerComponent.h>
#include <Scene/MysqlComponent.h>
#include <Scene/RpcConfigComponent.h>
#include <Scene/RedisComponent.h>
#include <Scene/MysqlProxyComponent.h>
#include <Scene/HttpComponent.h>
#include"Service/ProxyService.h"
#include <Telnet/TelnetClientComponent.h>

#include <Service/LuaServiceMgrComponent.h>
#include <ProtoRpc/ProtoRpcComponent.h>
#include <Service/NodeProxyComponent.h>
#include <Scene/TaskPoolComponent.h>
#include <ProtoRpc/ProtoRpcClientComponent.h>
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
#include"ProxyRpc/ProtoProxyComponent.h"
#include"ProxyRpc/ProtoProxyClientComponent.h"
using namespace GameKeeper;

int main(int argc, char **argv)
{
#ifdef __DEBUG__ && _WIN32
	system("chcp 936");
#endif // __DEBUF__ && _WIN32
    __register_component__(TimerComponent);
    __register_component__(ProtoRpcComponent);
    __register_component__(RedisComponent);
    __register_component__(MysqlComponent);
    __register_component__(LuaScriptComponent);
    __register_component__(RpcConfigComponent);
    __register_component__(MysqlProxyComponent);
    __register_component__(HttpComponent);
    __register_component__(OperatorComponent);
	__register_component__(LoggerComponent);

    __register_component__(TaskPoolComponent);
    __register_component__(CoroutineComponent);
    __register_component__(NodeProxyComponent);
    __register_component__(TcpServerComponent);
    __register_component__(ProtoRpcClientComponent);
    __register_component__(TelnetClientComponent);
    __register_component__(MonitorComponent);

    __register_component__(LuaServiceMgrComponent);
    __register_component__(ProtoProxyComponent);
    __register_component__(ProtoProxyClientComponent);

    __register_component__(MysqlService);
    __register_component__(ProxyService);
    __register_component__(AccountService);
    __register_component__(CenterHostService);
    __register_component__(LocalHostService);
    __register_component__(HttpLoginService);
    __register_component__(HttpOperComponent);
    __register_component__(HttpResourceComponent);

	if (argc == 1)
	{
		argv[1] = new char[100];
		argv[1] = "./Config/server.json";
	}


    App app;
    return app.Run(argc, argv);
}