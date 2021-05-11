#include<Core/Applocation.h>
#include<Core/ManagerFactory.h>
#include<Manager/ScriptManager.h>
#include<Manager/TimerManager.h>
#include<Manager/LocalActionManager.h>
#include<Manager/ActionRegisterManager.h>
#include<Manager/NetWorkManager.h>
#include<Manager/CommandManager.h>
#include<Manager/ListenerManager.h>
#include<Manager/RemoteActionManager.h>
#include<Manager/MysqlManager.h>
#include<Manager/RedisManager.h>
#include<Coroutine/CoroutineManager.h>
#include<Manager/ProxyManager.h>
using namespace SoEasy;
using namespace DataBase;

TYPE_REFLECTION(SoEasy::ProxyManager, "ProxyManager");
TYPE_REFLECTION(SoEasy::TimerManager, "TimerManager");
TYPE_REFLECTION(SoEasy::ScriptManager, "ScriptManager");
TYPE_REFLECTION(SoEasy::LocalActionManager, "LocalActionManager");
TYPE_REFLECTION(SoEasy::NetWorkManager, "NetWorkManager");
TYPE_REFLECTION(SoEasy::CommandManager, "CommandManager");
TYPE_REFLECTION(SoEasy::ListenerManager, "ListenerManager");
TYPE_REFLECTION(SoEasy::CoroutineManager, "CoroutineManager");
TYPE_REFLECTION(SoEasy::RemoteActionManager, "RemoteActionManager");
TYPE_REFLECTION(SoEasy::ActionRegisterManager, "ActionRegisterManager");

TYPE_REFLECTION(DataBase::RedisManager, "RedisManager");
TYPE_REFLECTION(DataBase::MysqlManager, "MysqlManager");


#ifdef _WIN32
#pragma comment(lib,"lua53.lib")
#pragma comment(lib,"Common.lib")
#pragma comment(lib,"hiredis.lib")
#pragma comment(lib,"libmysql.lib")
#pragma comment(lib,"DataBase.lib")
#pragma comment(lib,"CoreLogic.lib")
#pragma comment(lib,"ServerData.lib")
#pragma comment(lib,"libprotobufd.lib")
#endif
int main(int argc, char ** argv)
{ 
	ManagerFactory factory;
	factory.RegisterManager<ProxyManager>();
	factory.RegisterManager<RedisManager>();
	factory.RegisterManager<MysqlManager>();
	factory.RegisterManager<TimerManager>();
	factory.RegisterManager<ScriptManager>();
	factory.RegisterManager<LocalActionManager>();
	factory.RegisterManager<NetWorkManager>();
	factory.RegisterManager<CommandManager>();
	factory.RegisterManager<ListenerManager>();
	factory.RegisterManager<CoroutineManager>();
	factory.RegisterManager<RemoteActionManager>();
	factory.RegisterManager<ActionRegisterManager>();

	std::string serverName = argc == 3 ? argv[1] : "Server";
	std::string configPath = argc == 3 ? argv[2] : "./Config/ServerConfig.json";
	
	Applocation app(serverName, factory, configPath);

	return app.Run();
}