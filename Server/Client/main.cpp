#include<Core/Applocation.h>
#include<Object/ObjectRegistry.h>
#include<Manager/ScriptManager.h>
#include<Manager/TimerManager.h>
#include<Manager/NetSessionManager.h>
#include<Manager/ActionManager.h>
#include<Manager/ServiceManager.h>
#include<Manager/ServiceNodeManager.h>
#include"Manager/ClientManager.h"
#include<Coroutine/CoroutineManager.h>
#include<Manager/NetProxyManager.h>

using namespace Sentry;
using namespace Client;

#ifdef _WIN32
#pragma comment(lib,"lua53.lib")
#pragma comment(lib,"Common.lib")
#pragma comment(lib,"ServerData.lib")
#pragma comment(lib,"libprotobufd.lib")
#endif
int main(int argc, char ** argv)
{
	ObjectRegistry<Manager>::Register<TimerManager>("TimerManager");
	ObjectRegistry<Manager>::Register<ClientManager>("ClientManager");
	ObjectRegistry<Manager>::Register<ScriptManager>("ScriptManager");
	ObjectRegistry<Manager>::Register<ActionManager>("ActionManager");
	ObjectRegistry<Manager>::Register<ServiceManager>("ServiceManager");
	ObjectRegistry<Manager>::Register<NetProxyManager>("NetProxyManager");
	ObjectRegistry<Manager>::Register<CoroutineManager>("CoroutineManager");
	ObjectRegistry<Manager>::Register<NetSessionManager>("NetSessionManager");
	ObjectRegistry<Manager>::Register<ServiceNodeManager>("ServiceNodeManager");

	


	Applocation app("Client", "./Config/ClientConfig.json");

	return app.Run();
}