#include<Core/Applocation.h>
#include<Core/ManagerFactory.h>
#include<Manager/ScriptManager.h>
#include<Manager/TimerManager.h>
#include<Manager/NetWorkManager.h>
#include<Manager/LocalActionManager.h>
#include<Coroutine/CoroutineManager.h>
#include"Manager/ClientManager.h"
using namespace SoEasy;
using namespace Client;

TYPE_REFLECTION(Client::ClientManager, "ClientManager");
TYPE_REFLECTION(SoEasy::TimerManager, "TimerManager");
TYPE_REFLECTION(SoEasy::LocalActionManager, "LocalActionManager");
TYPE_REFLECTION(SoEasy::ScriptManager, "ScriptManager");
TYPE_REFLECTION(SoEasy::NetWorkManager, "NetWorkManager");
TYPE_REFLECTION(SoEasy::CoroutineManager, "CoroutineManager");


#ifdef _WIN32
#pragma comment(lib,"lua53.lib")
#pragma comment(lib,"Common.lib")
#pragma comment(lib,"ServerData.lib")
#pragma comment(lib,"libprotobufd.lib")
#endif
int main(int argc, char ** argv)
{
	ManagerFactory factory;
	factory.RegisterManager<LocalActionManager>();
	factory.RegisterManager<ClientManager>();
	factory.RegisterManager<TimerManager>();
	factory.RegisterManager<ScriptManager>();
	factory.RegisterManager<NetWorkManager>();
	factory.RegisterManager<CoroutineManager>();


	Applocation app("Client", factory, "./Config/ClientConfig.json");

	return app.Run();
}