#ifdef JE_MALLOC
#include"jemalloc/jemalloc.h"
#endif
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
#include"Service/GateService.h"
#include <Telnet/TelnetClientComponent.h>

#include "Component/Scene/LuaServiceMgrComponent.h"
#include <Rpc/RpcComponent.h>
#include "Component/Scene/RpcNodeComponent.h"
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

void RegisterComponent()
{
// rpc
    ComponentFactory::Add<RpcComponent>("RpcComponent");
    ComponentFactory::Add<RpcNodeComponent>("RpcNodeComponent");
    ComponentFactory::Add<RpcConfigComponent>("RpcConfigComponent");

// common
    ComponentFactory::Add<TaskComponent>("TaskComponent");
    ComponentFactory::Add<TimerComponent>("TimerComponent");
    ComponentFactory::Add<LoggerComponent>("LoggerComponent");
    ComponentFactory::Add<MonitorComponent>("MonitorComponent");
    ComponentFactory::Add<OperatorComponent>("OperatorComponent");
    ComponentFactory::Add<ThreadPoolComponent>("ThreadPoolComponent");

//server
    ComponentFactory::Add<TcpServerComponent>("TcpServerComponent");
    ComponentFactory::Add<RpcClientComponent>("RpcClientComponent");
    ComponentFactory::Add<TelnetClientComponent>("TelnetClientComponent");

// gate
    ComponentFactory::Add<GateComponent>("GateComponent");
    ComponentFactory::Add<GateClientComponent>("GateClientComponent");

// db
    ComponentFactory::Add<RedisComponent>("RedisComponent");
    ComponentFactory::Add<MysqlComponent>("MysqlComponent");
    ComponentFactory::Add<MysqlProxyComponent>("MysqlProxyComponent");

//http
    ComponentFactory::Add<HttpClientComponent>("HttpClientComponent");
// lua
    ComponentFactory::Add<LuaScriptComponent>("LuaScriptComponent");
    ComponentFactory::Add<LuaServiceMgrComponent>("LuaServiceMgrComponent");
}

void RegisterServiceComponent()
{
    ComponentFactory::Add<GateService>("GateService");
    ComponentFactory::Add<MysqlService>("MysqlService");
    ComponentFactory::Add<AccountService>("AccountService");
    ComponentFactory::Add<LocalHostService>("LocalHostService");
    ComponentFactory::Add<HttpLoginService>("HttpLoginService");
    ComponentFactory::Add<HttpOperComponent>("HttpOperComponent");
    ComponentFactory::Add<CenterHostService>("CenterHostService");
}

int main(int argc, char **argv)
{
    try
    {
        RegisterComponent();
        RegisterServiceComponent();
        const std::string path(argv[1]);
        ServerConfig * serverConfig = new ServerConfig(path);
        return (new App(serverConfig))->Run(argc, argv);
    }
    catch(std::logic_error & err)
    {
        std::cerr << err.what() << std::endl;
        return -1;
    }
    return -1;
}