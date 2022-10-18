#include"App/App.h"
#include"Component/TimerComponent.h"
#include"Component/TaskComponent.h"
#include"Component/LuaScriptComponent.h"
#include"Component/RedisDataComponent.h"
#include"Component/UnitMgrComponent.h"
#include"Component/ConsoleComponent.h"
#include"Component/NetThreadComponent.h"
#include"Component/RedisRegistryComponent.h"
#include"Component/OperatorComponent.h"
#include"Component/LoggerComponent.h"
#include"Component/ProtoComponent.h"

#include"Component/InnerNetComponent.h"
#include"Component/OuterNetComponent.h"
#include"Component/OuterNetMessageComponent.h"
#include"Component/InnerNetMessageComponent.h"

#include"Component/GateHelperComponent.h"
#include"Component/UserSyncComponent.h"
#include"Component/ClientComponent.h"
#include"Component/RedisSubComponent.h"


#include"Component/HttpComponent.h"
#include"Component/HttpWebComponent.h"
#include"Component/LocationComponent.h"
#include"Component/TextConfigComponent.h"
#include"Component/ClusterComponent.h"
#include"Component/ForwardComponent.h"
#include"Component/ForwardHelperComponent.h"

#include"Service/LuaRpcService.h"
#include"Service/OuterService.h"
#include"Service/InnerService.h"
#include"Service/ServiceRpcComponent.h"
#include"Service/HttpWebService.h"
#include"Service/HttpRpcService.h"
#include"Service/UserBehavior.h"
#include"Service/LocalLuaHttpService.h"
#ifdef __ENABLE_MONGODB__
#include"Service/MongoService.h"
#include"Component/MongoDBComponent.h"
#include"Component/DataSyncComponent.h"
#include"Component/MongoHelperComponent.h"
#include"Component/MongoDataComponent.h"
#endif

#ifdef __ENABLE_MYSQL__
#include"Service/MysqlService.h"
#include"Component/MysqlDBComponent.h"
#include"Component/MysqlHelperComponent.h"
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
    ComponentFactory::Add<ClusterComponent>("ClusterComponent");
    ComponentFactory::Add<TextConfigComponent>("TextConfigComponent");
	ComponentFactory::Add<ConsoleComponent>("ConsoleComponent");
    ComponentFactory::Add<InnerNetComponent>("InnerNetComponent");
	ComponentFactory::Add<LocationComponent>("LocationComponent");

    ComponentFactory::Add<ForwardComponent>("ForwardComponent");
    ComponentFactory::Add<ForwardHelperComponent>("ForwardHelperComponent");

// gate
	ComponentFactory::Add<GateHelperComponent>("GateHelperComponent");
	ComponentFactory::Add<OuterNetComponent>("OuterNetComponent");
	ComponentFactory::Add<OuterNetMessageComponent>("OuterNetMessageComponent");
// db
    ComponentFactory::Add<RedisDataComponent>("RedisDataComponent");
    ComponentFactory::Add<RedisSubComponent>("RedisSubComponent");

#ifdef __ENABLE_MONGODB__
    ComponentFactory::Add<MongoDBComponent>("MongoDBComponent");
    ComponentFactory::Add<DataSyncComponent>("DataSyncComponent");
    ComponentFactory::Add<MongoDataComponent>("MongoDataComponent");
    ComponentFactory::Add<MongoHelperComponent>("MongoHelperComponent");
#endif

#ifdef __ENABLE_MYSQL__
    ComponentFactory::Add<MysqlDBComponent>("MysqlDBComponent");
    ComponentFactory::Add<MysqlHelperComponent>("MysqlHelperComponent");
#endif
//http
    ComponentFactory::Add<HttpComponent>("HttpComponent");
    ComponentFactory::Add<HttpWebComponent>("HttpWebComponent");

    ComponentFactory::Add<HttpRpcService>("HttpRpcService");
    ComponentFactory::Add<HttpWebService>("HttpWebService");

// lua
    ComponentFactory::Add<LuaScriptComponent>("LuaScriptComponent");
    ComponentFactory::Add<Client::ClientComponent>("ClientComponent");

}

void RegisterServiceComponent()
{
    ComponentFactory::Add<LuaRpcService>("rpc");
    ComponentFactory::Add<ServiceRpcComponent>("agent");
    ComponentFactory::Add<LocalLuaHttpService>("http");
    ComponentFactory::Add<UserBehavior>("UserBehavior");
    ComponentFactory::Add<OuterService>("OuterService");
    ComponentFactory::Add<InnerService>("InnerService");
#ifdef __ENABLE_MONGODB__
	ComponentFactory::Add<MongoService>("MongoService");
#endif

#ifdef __ENABLE_MYSQL__
    ComponentFactory::Add<MysqlService>("MysqlService");
#endif
}
int main(int argc, char **argv)
{
    if(argc != 3)
    {
        CONSOLE_LOG_ERROR("start argc error");
        return 1;
    }
	RegisterComponent();
	RegisterServiceComponent();
	return (new App())->Run(argc, argv);
}