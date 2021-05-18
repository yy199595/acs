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
#include<Manager/LoginManager.h>
#include<Manager/UserDataManager.h>
#include<Manager/LocalAccessManager.h>
using namespace SoEasy;
using namespace SoEasy;

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
	factory.RegisterManager<LoginManager>("LoginManager");
	factory.RegisterManager<ProxyManager>("ProxyManager");
	factory.RegisterManager<RedisManager>("RedisManager");
	factory.RegisterManager<MysqlManager>("MysqlManager");
	factory.RegisterManager<TimerManager>("TimerManager");
	factory.RegisterManager<ScriptManager>("ScriptManager");
	factory.RegisterManager<NetWorkManager>("NetWorkManager");
	factory.RegisterManager<CommandManager>("CommandManager");
	factory.RegisterManager<ListenerManager>("ListenerManager");
	factory.RegisterManager<UserDataManager>("UserDataManager");
	factory.RegisterManager<CoroutineManager>("CoroutineManager");
	factory.RegisterManager<LocalAccessManager>("LocalAccessManager");
	factory.RegisterManager<LocalActionManager>("LocalActionManager");
	factory.RegisterManager<RemoteActionManager>("RemoteActionManager");
	factory.RegisterManager<ActionRegisterManager>("ActionRegisterManager");

	std::string serverName = argc == 3 ? argv[1] : "Server";
	std::string configPath = argc == 3 ? argv[2] : "./Config/ServerConfig.json";
	
	Applocation app(serverName, factory, configPath);

	return app.Run();
}