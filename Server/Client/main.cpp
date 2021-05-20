#include<Core/Applocation.h>
#include<Core/ManagerRegistry.h>
#include<Manager/ScriptManager.h>
#include<Manager/TimerManager.h>
#include<Manager/NetWorkManager.h>
#include<Manager/ActionManager.h>
#include<Coroutine/CoroutineManager.h>
#include"Manager/ClientManager.h"
using namespace SoEasy;
using namespace Client;

#ifdef _WIN32
#pragma comment(lib,"lua53.lib")
#pragma comment(lib,"Common.lib")
#pragma comment(lib,"ServerData.lib")
#pragma comment(lib,"libprotobufd.lib")
#endif
int main(int argc, char ** argv)
{
	
	ManagerRegistry::RegisterManager<TimerManager>("TimerManager");
	ManagerRegistry::RegisterManager<ClientManager>("ClientManager");	
	ManagerRegistry::RegisterManager<ScriptManager>("ScriptManager");
	ManagerRegistry::RegisterManager<NetWorkManager>("NetWorkManager");
	ManagerRegistry::RegisterManager<CoroutineManager>("CoroutineManager");
	ManagerRegistry::RegisterManager<ActionManager>("ActionManager");


	Applocation app("Client", "./Config/ClientConfig.json");

	return app.Run();
}