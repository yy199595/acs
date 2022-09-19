#include"App/App.h"
#include"Component/Timer/TimerComponent.h"
#include"Component/Coroutine/TaskComponent.h"
#include"Component/Lua/LuaScriptComponent.h"

#include"Network/Listener/TcpServerComponent.h"
#include"Global/ServiceConfig.h"
#include"Component/Redis/RedisDataComponent.h"
#include"Component/Scene/UnitMgrComponent.h"
#include"Component/Http/HttpComponent.h"
#include"Component/Gate/GateService.h"
#include"Component/Telnet/ConsoleComponent.h"
#include"Component/Rpc/InnerNetMessageComponent.h"
#include"Component/Scene/NetThreadComponent.h"
#include"Component/Rpc/InnerNetComponent.h"
#include"Component/Logic/RedisRegistryComponent.h"
#include"Component/Scene/OperatorComponent.h"
#include"Component/Scene/LoggerComponent.h"
#include"Component/Gate/OuterNetMessageComponent.h"
#include"Component/Gate/OuterNetComponent.h"
#include"Component/Scene/ProtoComponent.h"
#include"Component/RpcService/LuaService.h"
#include"Component/Http/HttpWebComponent.h"

#include"Component/HttpService/LocalLuaHttpService.h"
#include"Component/Mysql/MysqlDBComponent.h"


#include"Component/Gate/GateAgentComponent.h"
#include"Component/User/UserSyncComponent.h"
#include"Component/ClientComponent.h"

#include"Component/Redis/RedisSubComponent.h"
#include"Component/Http/HttpWebComponent.h"
#include"Component/Http/HttpRpcComponent.h"
#include"Component/RpcService/ServiceAgent.h"

#include"Component/HttpService/HttpWebService.h"

#ifdef __ENABLE_MONGODB__
#include"Service/MongoService.h"
#include"Component/MongoDBComponent.h"
#include"Component/DataSyncComponent.h"
#include"Component/MongoAgentComponent.h"
#include"Component/MongoDataComponent.h"
#endif
using namespace Sentry;
void RegisterComponent()
{
// rpc
    ComponentFactory::Add<RedisRegistryComponent>("RedisRegistryComponent");
    ComponentFactory::Add<InnerNetMessageComponent>("InnerNetMessageComponent");
// common
    ComponentFactory::Add<TaskComponent>("TaskComponent");
    ComponentFactory::Add<TimerComponent>("TimerComponent");
    ComponentFactory::Add<LoggerComponent>("LoggerComponent");
	ComponentFactory::Add<UserSyncComponent>("UserSyncComponent");
	ComponentFactory::Add<OperatorComponent>("OperatorComponent");
    ComponentFactory::Add<UnitMgrComponent>("UnitMgrComponent");
    ComponentFactory::Add<NetThreadComponent>("NetThreadComponent");
    ComponentFactory::Add<ProtoComponent>("ProtoComponent");

//server
	ComponentFactory::Add<ConsoleComponent>("ConsoleComponent");
    ComponentFactory::Add<TcpServerComponent>("TcpServerComponent");
    ComponentFactory::Add<InnerNetComponent>("InnerNetComponent");

// gate
	ComponentFactory::Add<GateAgentComponent>("GateAgentComponent");
	ComponentFactory::Add<OuterNetComponent>("OuterNetComponent");
	ComponentFactory::Add<OuterNetMessageComponent>("OuterNetMessageComponent");
// db
    ComponentFactory::Add<MysqlDBComponent>("MysqlDBComponent");
    ComponentFactory::Add<RedisDataComponent>("RedisDataComponent");
    ComponentFactory::Add<RedisSubComponent>("RedisSubComponent");

#ifdef __ENABLE_MONGODB__
    ComponentFactory::Add<MongoDBComponent>("MongoDBComponent");
    ComponentFactory::Add<DataSyncComponent>("DataSyncComponent");
    ComponentFactory::Add<MongoDataComponent>("MongoDataComponent");
    ComponentFactory::Add<MongoAgentComponent>("MongoAgentComponent");
#endif
//http
    ComponentFactory::Add<HttpComponent>("HttpComponent");
    ComponentFactory::Add<HttpWebComponent>("HttpWebComponent");
    ComponentFactory::Add<HttpRpcComponent>("HttpRpcComponent");
    ComponentFactory::Add<HttpWebService>("HttpWebService");

// lua
    ComponentFactory::Add<LuaScriptComponent>("LuaScriptComponent");
    ComponentFactory::Add<Client::ClientComponent>("ClientComponent");

}

void RegisterServiceComponent()
{
    ComponentFactory::Add<LuaService>("rpc");
    ComponentFactory::Add<ServiceAgent>("agent");
    ComponentFactory::Add<LocalLuaHttpService>("http");
    ComponentFactory::Add<GateService>("GateService");
#ifdef __ENABLE_MONGODB__
	ComponentFactory::Add<MongoService>("MongoService");
#endif
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