#include"App/App.h"
#include"Component/Timer/TimerComponent.h"
#include"Component/Coroutine/TaskComponent.h"
#include"Component/Lua/LuaScriptComponent.h"

#include<Network/Listener/TcpServerComponent.h>
#include"Global/ServiceConfig.h"
#include"Component/Redis/MainRedisComponent.h"
#include"Component/Mysql/MysqlProxyComponent.h"
#include"Component/Scene/EntityMgrComponent.h"
#include"Component/Http/HttpComponent.h"
#include"Component/Gate/GateService.h"
#include<Component/Telnet/ConsoleComponent.h>
#include<Component/Rpc/RpcHandlerComponent.h>
#include<Component/Scene/ThreadPoolComponent.h>
#include<Component/Rpc/RpcClientComponent.h>
#include"Component/Logic/HttpUserService.h"
#include"Component/Logic/RegistryService.h"
#include<Component/Scene/MonitorComponent.h>
#include"Component/Mysql/MysqlService.h"
#include"Component/Scene/OperatorComponent.h"
#include"Component/Scene/LoggerComponent.h"
#include"Component/Gate/GateComponent.h"
#include"Component/Gate/GateClientComponent.h"
#include"Component/ClientComponent.h"

#include"Component/Lua/LuaRegisterComponent.h"

#include"Component/Service/UserInfoSyncService.h"
#include"Component/Common/DataMgrComponent.h"
#include"Component/Gate/GateProxyComponent.h"
#include"Component/Redis/DataRedisComponent.h"
#include"Component/User/UserSyncComponent.h"
using namespace Sentry;
using namespace Client;

void RegisterComponent()
{
// rpc
    ComponentFactory::Add<RpcHandlerComponent>("RpcHandlerComponent");
// common
    ComponentFactory::Add<TaskComponent>("TaskComponent");
    ComponentFactory::Add<TimerComponent>("TimerComponent");
    ComponentFactory::Add<LoggerComponent>("LoggerComponent");
	ComponentFactory::Add<MonitorComponent>("MonitorComponent");
	ComponentFactory::Add<DataMgrComponent>("DataMgrComponent");
	ComponentFactory::Add<UserSyncComponent>("UserSyncComponent");
	ComponentFactory::Add<OperatorComponent>("OperatorComponent");
    ComponentFactory::Add<EntityMgrComponent>("EntityMgrComponent");
    ComponentFactory::Add<ThreadPoolComponent>("ThreadPoolComponent");

//server
	ComponentFactory::Add<ConsoleComponent>("ConsoleComponent");
    ComponentFactory::Add<TcpServerComponent>("TcpServerComponent");
    ComponentFactory::Add<RpcClientComponent>("RpcClientComponent");

// gate
    ComponentFactory::Add<GateComponent>("GateComponent");
	ComponentFactory::Add<GateProxyComponent>("GateProxyComponent");
	ComponentFactory::Add<GateClientComponent>("GateClientComponent");
// db
    ComponentFactory::Add<MainRedisComponent>("MainRedisComponent");
	ComponentFactory::Add<DataRedisComponent>("DataRedisComponent");
	ComponentFactory::Add<MysqlProxyComponent>("MysqlProxyComponent");

//http
    ComponentFactory::Add<HttpComponent>("HttpComponent");
// lua
    ComponentFactory::Add<LuaScriptComponent>("LuaScriptComponent");
	ComponentFactory::Add<LuaRegisterComponent>("LuaRegisterComponent");
	//client
	ComponentFactory::Add<ClientComponent>("ClientComponent");
}

void RegisterServiceComponent()
{
    ComponentFactory::Add<GateService>("GateService");
	ComponentFactory::Add<MysqlService>("MysqlService");
	ComponentFactory::Add<UserInfoSyncService>("UserInfoSyncService");
    ComponentFactory::Add<RegistryService>("RegistryService");
	ComponentFactory::Add<HttpUserService>("HttpUserService");
}
#include"Any/Any.h"

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