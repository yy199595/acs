#include<Core/Applocation.h>
#include<Core/ObjectRegistry.h>
#include<Manager/ScriptManager.h>
#include<Manager/TimerManager.h>
#include<Manager/ActionManager.h>

#include<Manager/NetWorkManager.h>
#include<Manager/ConsoleManager.h>
#include<Manager/ListenerManager.h>
#include<Manager/ServiceManager.h>
#include<Manager/MysqlManager.h>
#include<Manager/RedisManager.h>
#include<Coroutine/CoroutineManager.h>
#include<Manager/ProxyManager.h>
#include<Manager/LoginService.h>
#include<Manager/LocalAccessManager.h>


#include<Service/ServiceRegistry.h>
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
	
	
	ObjectRegistry<Manager>::Register<ProxyManager>("ProxyManager");
	ObjectRegistry<Manager>::Register<RedisManager>("RedisManager");
	ObjectRegistry<Manager>::Register<MysqlManager>("MysqlManager");
	ObjectRegistry<Manager>::Register<TimerManager>("TimerManager");
	ObjectRegistry<Manager>::Register<ActionManager>("ActionManager");
	ObjectRegistry<Manager>::Register<ScriptManager>("ScriptManager");
	ObjectRegistry<Manager>::Register<ServiceManager>("ServiceManager");
	ObjectRegistry<Manager>::Register<NetWorkManager>("NetWorkManager");
	ObjectRegistry<Manager>::Register<ConsoleManager>("ConsoleManager");
	ObjectRegistry<Manager>::Register<ListenerManager>("ListenerManager");
	ObjectRegistry<Manager>::Register<CoroutineManager>("CoroutineManager");
	ObjectRegistry<Manager>::Register<LocalAccessManager>("LocalAccessManager");

	ObjectRegistry<LocalService>::Register<LoginService>("LoginService");
	ObjectRegistry<LocalService>::Register<ServiceRegistry>("ServiceRegistry");

	std::string serverName = argc == 3 ? argv[1] : "Server";
	std::string configPath = argc == 3 ? argv[2] : "./Config/ServerConfig.json";
	
	Applocation app(serverName, configPath);

	return app.Run();
}