#include"App/App.h"
#include"Component/Timer/TimerComponent.h"
#include"Component/Coroutine/TaskComponent.h"
#include"Component/Lua/LuaScriptComponent.h"

#include<Network/Listener/TcpServerComponent.h>
#include"Global/ServiceConfig.h"
#include"Component/Redis/MainRedisComponent.h"
#include"Component/Mysql/MysqlAgentComponent.h"
#include"Component/Scene/EntityMgrComponent.h"
#include"Component/Http/HttpComponent.h"
#include"Component/Gate/GateService.h"
#include<Component/Telnet/ConsoleComponent.h>
#include<Component/Rpc/RpcHandlerComponent.h>
#include<Component/Scene/NetThreadComponent.h>
#include<Component/Rpc/RpcClientComponent.h>
#include"Component/Logic/ServiceMgrComponent.h"
#include"Component/Mysql/MysqlService.h"
#include"Component/Scene/OperatorComponent.h"
#include"Component/Scene/LoggerComponent.h"
#include"Component/Gate/GateComponent.h"
#include"Component/Gate/GateClientComponent.h"
#include"Component/Mongo/MongoService.h"
#include"Component/Scene/MessageComponent.h"
#include"Component/Logic/HttpSourceService.h"
#include"Component/RpcService/LocalLuaService.h"
#include"Component/Http/HttpHandlerComponent.h"
#include"Component/HttpService/LocalLuaHttpService.h"
#ifdef __ENABLE_CLIENT__
#include"Component/ClientComponent.h"
using namespace Client;
#endif
#include"Component/Common/DataMgrComponent.h"
#include"Component/Gate/GateAgentComponent.h"
#include"Component/User/UserSyncComponent.h"
using namespace Sentry;
void RegisterComponent()
{
// rpc
    ComponentFactory::Add<RpcHandlerComponent>("RpcHandlerComponent");
	ComponentFactory::Add<ServiceMgrComponent>("ServiceMgrComponent");
// common
    ComponentFactory::Add<TaskComponent>("TaskComponent");
    ComponentFactory::Add<TimerComponent>("TimerComponent");
    ComponentFactory::Add<LoggerComponent>("LoggerComponent");
	ComponentFactory::Add<DataMgrComponent>("DataMgrComponent");
	ComponentFactory::Add<UserSyncComponent>("UserSyncComponent");
	ComponentFactory::Add<OperatorComponent>("OperatorComponent");
    ComponentFactory::Add<EntityMgrComponent>("EntityMgrComponent");
    ComponentFactory::Add<NetThreadComponent>("NetThreadComponent");
    ComponentFactory::Add<MessageComponent>("MessageComponent");

//server
	ComponentFactory::Add<ConsoleComponent>("ConsoleComponent");
    ComponentFactory::Add<TcpServerComponent>("TcpServerComponent");
    ComponentFactory::Add<RpcClientComponent>("RpcClientComponent");

// gate
    ComponentFactory::Add<GateComponent>("GateComponent");
	ComponentFactory::Add<GateAgentComponent>("GateAgentComponent");
	ComponentFactory::Add<GateClientComponent>("GateClientComponent");
// db
    ComponentFactory::Add<MainRedisComponent>("MainRedisComponent");
	ComponentFactory::Add<MysqlAgentComponent>("MysqlAgentComponent");

//http
    ComponentFactory::Add<HttpComponent>("HttpComponent");
    ComponentFactory::Add<HttpHandlerComponent>("HttpHandlerComponent");

// lua
    ComponentFactory::Add<LuaScriptComponent>("LuaScriptComponent");
#ifdef __ENABLE_CLIENT__
	ComponentFactory::Add<ClientComponent>("ClientComponent");
#endif
}

void RegisterServiceComponent()
{
    ComponentFactory::Add<LocalLuaService>("rpc");
    ComponentFactory::Add<LocalLuaHttpService>("http");
    ComponentFactory::Add<GateService>("GateService");
	ComponentFactory::Add<MysqlService>("MysqlService");
	ComponentFactory::Add<MongoService>("MongoService");
	ComponentFactory::Add<HttpSourceService>("HttpSourceService");
}
int main(int argc, char **argv)
{
	try
    {
        RegisterComponent();
        RegisterServiceComponent();
        return (std::make_shared<App>(new ServerConfig(argc, argv)))->Run();
    }
    catch(std::logic_error & err)
    {
        std::cerr << err.what() << std::endl;
        return -1;
    }
}