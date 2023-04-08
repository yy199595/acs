#include"Entity/App/App.h"
#include"Timer/Component/TimerComponent.h"
#include"Async/Component/AsyncMgrComponent.h"
#include"Script/Component/LuaScriptComponent.h"
#include"Entity/Component/UnitMgrComponent.h"
#include"Telnet/Component/ConsoleComponent.h"
#include"Server/Component/ThreadComponent.h"
#include"Log/Component/LogComponent.h"
#include"Proto/Component/ProtoComponent.h"

#include"Rpc/Component/InnerNetComponent.h"
#include"Gate/Component/OuterNetComponent.h"
#include"Rpc/Component/InnerNetMessageComponent.h"

#include"Gate/Component/GateComponent.h"


#include"Http/Component/HttpComponent.h"
#include"Http/Component/HttpWebComponent.h"
#include"Http/Component/HttpDebugComponent.h"
#include"Rpc/Component/NodeMgrComponent.h"

#include"Server/Component/TextConfigComponent.h"
#include"Cluster/Component/LaunchComponent.h"
#include"Log/Service/Log.h"
#include"Gate/Service/Gate.h"
#include"Common/Service/Node.h"
#include"Http/Service/ServerWeb.h"
#include"Registry/Service/Registry.h"
#include"Common/Service/User.h"

#include"Component/ClientComponent.h"
#include"Kcp/Component/KcpComponent.h"
#ifdef __ENABLE_MONGODB__
#include"Mongo/Service/MongoDB.h"
#include"Mongo/Component/MongoDBComponent.h"
#include"Mongo/Component/MongoHelperComponent.h"
#endif

#ifdef __ENABLE_MYSQL__
#include"Mysql/Service/MysqlDB.h"
#include"Mysql/Component/MysqlDBComponent.h"
#include"Mysql/Component/MysqlHelperComponent.h"
#endif

#ifdef __ENABLE_REDIS__
#include"Redis/Component/RedisComponent.h"
#include"Redis/Component/RedisStringComponent.h"
#include"Redis/Component/RedisScriptComponent.h"
#endif
#include"Sqlite/Component/SqliteComponent.h"

#include"WatchDog/Service/WatchDog.h"
#include"WatchDog/Component/WatchDogComponent.h"
#include "Server/Config/CsvTextConfig.h"

using namespace Tendo;

void RegisterComponent()
{
// common
    ComponentFactory::Add<AsyncMgrComponent>("AsyncMgrComponent");
    ComponentFactory::Add<TimerComponent>("TimerComponent");
    ComponentFactory::Add<LogComponent>("LogComponent");
    ComponentFactory::Add<UnitMgrComponent>("UnitMgrComponent");
    ComponentFactory::Add<ThreadComponent>("ThreadComponent");
    ComponentFactory::Add<ProtoComponent>("ProtoComponent");

//server
	ComponentFactory::Add<KcpComponent>("KcpComponent");
    ComponentFactory::Add<LaunchComponent>("LaunchComponent");
    ComponentFactory::Add<TextConfigComponent>("TextConfigComponent");
	ComponentFactory::Add<ConsoleComponent>("ConsoleComponent");
    ComponentFactory::Add<InnerNetComponent>("InnerNetComponent");
	ComponentFactory::Add<NodeMgrComponent>("NodeMgrComponent");
	ComponentFactory::Add<InnerNetMessageComponent>("InnerNetMessageComponent");

// gate
	ComponentFactory::Add<GateComponent>("GateComponent");
	ComponentFactory::Add<OuterNetComponent>("OuterNetComponent");
    ComponentFactory::Add<SqliteComponent>("SqliteComponent");
// db
#ifdef __ENABLE_REDIS__
    ComponentFactory::Add<RedisComponent>("RedisComponent");
    ComponentFactory::Add<RedisStringComponent>("RedisStringComponent");
    ComponentFactory::Add<RedisScriptComponent>("RedisScriptComponent");
#endif

#ifdef __ENABLE_MONGODB__
    ComponentFactory::Add<MongoDBComponent>("MongoDBComponent");
    ComponentFactory::Add<MongoHelperComponent>("MongoHelperComponent");
#endif

#ifdef __ENABLE_MYSQL__
    ComponentFactory::Add<MysqlDBComponent>("MysqlDBComponent");
    ComponentFactory::Add<MysqlHelperComponent>("MysqlHelperComponent");
#endif
//http
    ComponentFactory::Add<HttpComponent>("HttpComponent");
    ComponentFactory::Add<HttpWebComponent>("HttpWebComponent");
    ComponentFactory::Add<HttpDebugComponent>("HttpDebugComponent");
  
    ComponentFactory::Add<ServerWeb>("ServerWeb");
    ComponentFactory::Add<WatchDogComponent>("WatchDogComponent");

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
    ComponentFactory::Add<WatchDog>("WatchDog");
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