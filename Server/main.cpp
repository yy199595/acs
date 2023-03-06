#include"App/App.h"
#include"Component/TimerComponent.h"
#include"Component/TaskComponent.h"
#include"Component/LuaScriptComponent.h"
#include"Component/UnitMgrComponent.h"
#include"Component/ConsoleComponent.h"
#include"Component/NetThreadComponent.h"
#include"Component/LogComponent.h"
#include"Component/ProtoComponent.h"

#include"Component/InnerNetComponent.h"
#include"Component/OuterNetComponent.h"
#include"Component/OuterNetMessageComponent.h"
#include"Component/InnerNetMessageComponent.h"

#include"Component/GateHelperComponent.h"
#include"Component/ClientComponent.h"

#include"Component/HttpComponent.h"
#include"Component/HttpWebComponent.h"
#include"Component/NodeMgrComponent.h"
#include"Component/TextConfigComponent.h"
#include"Component/LaunchComponent.h"
#include"Service/Log.h"
#include"Service/Gate.h"
#include"Service/Node.h"
#include"Service/HttpBackGround.h"
#include"Service/Registry.h"
#include"Service/HttpDebug.h"
#include"Service/User.h"
#ifdef __ENABLE_MONGODB__
#include"Service/MongoDB.h"
#include"Component/MongoDBComponent.h"
#include"Component/MongoHelperComponent.h"
#include"Component/MongoDataComponent.h"
#endif

#ifdef __ENABLE_MYSQL__
#include"Service/MysqlDB.h"
#include"Component/MysqlDBComponent.h"
#include"Component/MysqlHelperComponent.h"
#endif

#ifdef __ENABLE_REDIS__
#include"Component/RedisComponent.h"
#include"Component/RedisStringComponent.h"
#include"Component/RedisScriptComponent.h"
#endif
#include"Component/SqliteComponent.h"

using namespace Sentry;

void RegisterComponent()
{
// rpc
    ComponentFactory::Add<InnerNetMessageComponent>("InnerNetMessageComponent");
// common
    ComponentFactory::Add<TaskComponent>("TaskComponent");
    ComponentFactory::Add<TimerComponent>("TimerComponent");
    ComponentFactory::Add<LogComponent>("LogComponent");
    ComponentFactory::Add<UnitMgrComponent>("UnitMgrComponent");
    ComponentFactory::Add<NetThreadComponent>("NetThreadComponent");
    ComponentFactory::Add<ProtoComponent>("ProtoComponent");

//server
    ComponentFactory::Add<LaunchComponent>("LaunchComponent");
    ComponentFactory::Add<TextConfigComponent>("TextConfigComponent");
	ComponentFactory::Add<ConsoleComponent>("ConsoleComponent");
    ComponentFactory::Add<InnerNetComponent>("InnerNetComponent");
	ComponentFactory::Add<NodeMgrComponent>("NodeMgrComponent");

// gate
	ComponentFactory::Add<GateHelperComponent>("GateHelperComponent");
	ComponentFactory::Add<OuterNetComponent>("OuterNetComponent");
	ComponentFactory::Add<OuterNetMessageComponent>("OuterNetMessageComponent");
    ComponentFactory::Add<SqliteComponent>("SqliteComponent");
// db
#ifdef __ENABLE_REDIS__
    ComponentFactory::Add<RedisComponent>("RedisComponent");
    ComponentFactory::Add<RedisStringComponent>("RedisStringComponent");
    ComponentFactory::Add<RedisScriptComponent>("RedisScriptComponent");
#endif

#ifdef __ENABLE_MONGODB__
    ComponentFactory::Add<MongoDBComponent>("MongoDBComponent");
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

    ComponentFactory::Add<HttpDebug>("HttpDebug");
    ComponentFactory::Add<HttpBackGround>("HttpBackGround");

// lua
    ComponentFactory::Add<LuaScriptComponent>("LuaScriptComponent");
    ComponentFactory::Add<Client::ClientComponent>("ClientComponent");

}

void RegisterServiceComponent()
{
	ComponentFactory::Add<Log>("Log");
	ComponentFactory::Add<Gate>("Gate");
	ComponentFactory::Add<User>("User");
    ComponentFactory::Add<Node>("Node");
    ComponentFactory::Add<Registry>("Registry");
#ifdef __ENABLE_MONGODB__
    ComponentFactory::Add<MongoDB>("MongoDB");
#endif

#ifdef __ENABLE_MYSQL__
    ComponentFactory::Add<MysqlDB>("MysqlDB");
#endif
}
int main(int argc, char **argv)
{

#ifdef __OS_WIN__
    system("chcp 65001 > nul"); 
#endif
	RegisterComponent();
	RegisterServiceComponent();
	return (new App())->Run(argc, argv);
}