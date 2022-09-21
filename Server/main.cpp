#include"App/App.h"
#include"Component/TimerComponent.h"
#include"Component/TaskComponent.h"
#include"Component/LuaScriptComponent.h"

#include"Component/TcpServerComponent.h"
#include"Config/ServiceConfig.h"
#include"Component/RedisDataComponent.h"
#include"Component/UnitMgrComponent.h"
#include"Component/HttpComponent.h"
#include"Service/GateService.h"
#include"Component/ConsoleComponent.h"
#include"Component/InnerNetMessageComponent.h"
#include"Component/NetThreadComponent.h"
#include"Component/InnerNetComponent.h"
#include"Component/RedisRegistryComponent.h"
#include"Component/OperatorComponent.h"
#include"Component/LoggerComponent.h"
#include"Component/OuterNetMessageComponent.h"
#include"Component/OuterNetComponent.h"
#include"Component/ProtoComponent.h"
#include"Service/LuaService.h"
#include"Component/HttpWebComponent.h"

#include"Service/LocalLuaHttpService.h"

#include"Component/GateAgentComponent.h"
#include"Component/UserSyncComponent.h"
#include"Component/ClientComponent.h"

#include"Component/RedisSubComponent.h"
#include"Component/HttpRpcComponent.h"
#include"Service/ServiceAgent.h"

#include"Service/HttpWebService.h"

#ifdef __ENABLE_MONGODB__
#include"Service/MongoService.h"
#include"Component/MongoDBComponent.h"
#include"Component/DataSyncComponent.h"
#include"Component/MongoAgentComponent.h"
#include"Component/MongoDataComponent.h"
#endif

#ifdef __ENABLE_MYSQL__
#include"Service/MysqlService.h"
#include"Component/MysqlDBComponent.h"
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
    ComponentFactory::Add<RedisDataComponent>("RedisDataComponent");
    ComponentFactory::Add<RedisSubComponent>("RedisSubComponent");

#ifdef __ENABLE_MONGODB__
    ComponentFactory::Add<MongoDBComponent>("MongoDBComponent");
    ComponentFactory::Add<DataSyncComponent>("DataSyncComponent");
    ComponentFactory::Add<MongoDataComponent>("MongoDataComponent");
    ComponentFactory::Add<MongoAgentComponent>("MongoAgentComponent");
#endif

#ifdef __ENABLE_MYSQL__
    ComponentFactory::Add<MysqlDBComponent>("MysqlDBComponent");
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

#ifdef __ENABLE_MYSQL__
    ComponentFactory::Add<MysqlService>("MysqlService");
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