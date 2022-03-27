﻿#ifdef JE_MALLOC
#include"jemalloc/jemalloc.h"
#endif
#include"App/App.h"
#include"Component/Timer/TimerComponent.h"
#include"Component/Coroutine/TaskComponent.h"
#include"Component/Lua/LuaScriptComponent.h"

#include<Network/Listener/TcpServerComponent.h>
#include"Component/Mysql/MysqlComponent.h"
#include"Component/Rpc/RpcConfigComponent.h"
#include"Component/Redis/RedisComponent.h"
#include"Component/Mysql/MysqlProxyComponent.h"
#include"Component/Scene/EntityMgrComponent.h"
#include"Component/Http/HttpClientComponent.h"
#include"Component/Gate/GateService.h"
#include<Component/Telnet/ConsoleComponent.h>
#include"Component/Lua/LuaServiceMgrComponent.h"
#include<Component/Rpc/RpcComponent.h>
#include"Component/Scene/ServiceMgrComponent.h"
#include<Component/Scene/ThreadPoolComponent.h>
#include<Component/Rpc/RpcClientComponent.h>
#include"Component/Logic/HttpUserService.h"
#include"Component/Logic/LocalService.h"
#include<Component/Scene/MonitorComponent.h>
#include"Component/Mysql/MysqlService.h"
#include"Component/Logic/HttpLoginService.h"
#include"Component/Scene/OperatorComponent.h"
#include"Component/Scene/LoggerComponent.h"
#include"Component/Gate/GateComponent.h"
#include"Component/Gate/GateClientComponent.h"
#include"Component/Service/HttpNodeService.h"
#include"Component/ClientComponent.h"
using namespace Sentry;
using namespace Client;

void RegisterComponent()
{
// rpc
    ComponentFactory::Add<RpcComponent>("RpcComponent");
    ComponentFactory::Add<ServiceMgrComponent>("ServiceMgrComponent");
    ComponentFactory::Add<RpcConfigComponent>("RpcConfigComponent");

// common
    ComponentFactory::Add<TaskComponent>("TaskComponent");
    ComponentFactory::Add<TimerComponent>("TimerComponent");
    ComponentFactory::Add<LoggerComponent>("LoggerComponent");
    ComponentFactory::Add<MonitorComponent>("MonitorComponent");
    ComponentFactory::Add<OperatorComponent>("OperatorComponent");
    ComponentFactory::Add<EntityMgrComponent>("EntityMgrComponent");
    ComponentFactory::Add<ThreadPoolComponent>("ThreadPoolComponent");

//server
    ComponentFactory::Add<TcpServerComponent>("TcpServerComponent");
    ComponentFactory::Add<RpcClientComponent>("RpcClientComponent");
    ComponentFactory::Add<ConsoleComponent>("ConsoleComponent");

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
	//client
	ComponentFactory::Add<ClientComponent>("ClientComponent");
}

void RegisterServiceComponent()
{
    ComponentFactory::Add<GateService>("GateService");
    ComponentFactory::Add<LocalService>("LocalService");
    ComponentFactory::Add<MysqlService>("MysqlService");
    ComponentFactory::Add<HttpUserService>("HttpUserService");
    ComponentFactory::Add<HttpNodeService>("HttpNodeService");
    ComponentFactory::Add<HttpLoginService>("HttpLoginService");
}
#include"spdlog/fmt/bundled/color.h"

int main(int argc, char **argv)
{
	try
    {
        RegisterComponent();
        RegisterServiceComponent();
        const std::string path(argv[1]);
        return (std::make_shared<App>(new ServerConfig(path)))->Run();
    }
    catch(std::logic_error & err)
    {
        std::cerr << err.what() << std::endl;
        return -1;
    }
}