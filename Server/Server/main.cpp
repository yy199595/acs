#include<Core/Applocation.h>
#include<Core/ManagerRegistry.h>
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
	
	ManagerRegistry::RegisterManager<LoginManager>("LoginManager");
	ManagerRegistry::RegisterManager<ProxyManager>("ProxyManager");
	ManagerRegistry::RegisterManager<RedisManager>("RedisManager");
	ManagerRegistry::RegisterManager<MysqlManager>("MysqlManager");
	ManagerRegistry::RegisterManager<TimerManager>("TimerManager");
	ManagerRegistry::RegisterManager<ScriptManager>("ScriptManager");
	ManagerRegistry::RegisterManager<NetWorkManager>("NetWorkManager");
	ManagerRegistry::RegisterManager<CommandManager>("CommandManager");
	ManagerRegistry::RegisterManager<ListenerManager>("ListenerManager");
	ManagerRegistry::RegisterManager<UserDataManager>("UserDataManager");
	ManagerRegistry::RegisterManager<CoroutineManager>("CoroutineManager");
	ManagerRegistry::RegisterManager<LocalAccessManager>("LocalAccessManager");
	ManagerRegistry::RegisterManager<LocalActionManager>("LocalActionManager");
	ManagerRegistry::RegisterManager<RemoteActionManager>("RemoteActionManager");
	ManagerRegistry::RegisterManager<ActionRegisterManager>("ActionRegisterManager");

	std::string serverName = argc == 3 ? argv[1] : "Server";
	std::string configPath = argc == 3 ? argv[2] : "./Config/ServerConfig.json";
	
	Applocation app(serverName, configPath);

	return app.Run();
}