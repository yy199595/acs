#include<Core/Applocation.h>
#include<Core/ObjectRegistry.h>
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
	
	ObjectRegistry<Manager>::RegisterManager<TimerManager>("TimerManager");
	ObjectRegistry<Manager>::RegisterManager<ClientManager>("ClientManager");
	ObjectRegistry<Manager>::RegisterManager<ScriptManager>("ScriptManager");
	ObjectRegistry<Manager>::RegisterManager<NetWorkManager>("NetWorkManager");
	ObjectRegistry<Manager>::RegisterManager<CoroutineManager>("CoroutineManager");
	ObjectRegistry<Manager>::RegisterManager<ActionManager>("ActionManager");


	Applocation app("Client", "./Config/ClientConfig.json");

	return app.Run();
}