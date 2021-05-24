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
	
	
	ObjectRegistry<Manager>::RegisterManager<ProxyManager>("ProxyManager");
	ObjectRegistry<Manager>::RegisterManager<RedisManager>("RedisManager");
	ObjectRegistry<Manager>::RegisterManager<MysqlManager>("MysqlManager");
	ObjectRegistry<Manager>::RegisterManager<TimerManager>("TimerManager");
	ObjectRegistry<Manager>::RegisterManager<ActionManager>("ActionManager");
	ObjectRegistry<Manager>::RegisterManager<ScriptManager>("ScriptManager");
	ObjectRegistry<Manager>::RegisterManager<ServiceManager>("ServiceManager");
	ObjectRegistry<Manager>::RegisterManager<NetWorkManager>("NetWorkManager");
	ObjectRegistry<Manager>::RegisterManager<ConsoleManager>("ConsoleManager");
	ObjectRegistry<Manager>::RegisterManager<ListenerManager>("ListenerManager");
	ObjectRegistry<Manager>::RegisterManager<CoroutineManager>("CoroutineManager");
	ObjectRegistry<Manager>::RegisterManager<LocalAccessManager>("LocalAccessManager");

	ObjectRegistry<LocalService>::RegisterManager<LoginService>("LoginService");
	ObjectRegistry<LocalService>::RegisterManager<ServiceRegistry>("ServiceRegistry");

	std::string serverName = argc == 3 ? argv[1] : "Server";
	std::string configPath = argc == 3 ? argv[2] : "./Config/ServerConfig.json";
	
	Applocation app(serverName, configPath);

	return app.Run();
}