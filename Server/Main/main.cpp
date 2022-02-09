#ifdef JE_MALLOC
#include"jemalloc/jemalloc.h"
#endif
#include "Object/App.h"
#include <Scene/LuaScriptComponent.h>
#include <Timer/TimerComponent.h>
#include <Coroutine/TaskComponent.h>
#include <Listener/TcpServerComponent.h>
#include "MysqlComponent.h"
#include <Scene/RpcConfigComponent.h>
#include "RedisComponent.h"
#include "MysqlProxyComponent.h"
#include "Http/Component/HttpClientComponent.h"
#include"Service/GateService.h"
#include <Telnet/ConsoleComponent.h>

#include "Component/Scene/LuaServiceMgrComponent.h"
#include <Rpc/RpcComponent.h>
#include "Component/Scene/ServiceMgrComponent.h"
#include <Scene/ThreadPoolComponent.h>
#include <Rpc/RpcClientComponent.h>
#include <Service/AccountService.h>
#include "Component/Service/LocalService.h"
#include <Scene/MonitorComponent.h>
#include "Service/MysqlService.h"
#include <Service/HttpLoginService.h>
#include "Http/Service/HttpOperComponent.h"
#include "Scene/OperatorComponent.h"
#include"Scene/LoggerComponent.h"
#include"Component/GateComponent.h"
#include"Component/GateClientComponent.h"

#include"Service/HttpNodeService.h"
using namespace Sentry;

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
}

void RegisterServiceComponent()
{
    ComponentFactory::Add<GateService>("GateService");
    ComponentFactory::Add<LocalService>("LocalService");
    ComponentFactory::Add<MysqlService>("MysqlService");
    ComponentFactory::Add<AccountService>("AccountService");
    ComponentFactory::Add<HttpNodeService>("HttpNodeService");
    ComponentFactory::Add<HttpLoginService>("HttpLoginService");
    ComponentFactory::Add<HttpOperComponent>("HttpOperComponent");
}


int main(int argc, char **argv)
{
    try
    {
        asio::io_service io;
        asio::ip::tcp::resolver resolver(io);
        asio::ip::tcp::resolver::query query(asio::ip::host_name(), "");
        asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
        asio::ip::tcp::resolver::iterator end;

        std::set<std::string> address;
        while(iter != end)
        {
            tcp::endpoint ep = *iter++;
            address.emplace(ep.address().to_string());
        }

        for(const std::string & add : address)
        {
            std::cout << add << std::endl;
        }

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