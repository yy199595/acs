#include<CommonManager/TimerManager.h>
#include<CommonManager/NetWorkManager.h>
#include<CommonManager/ScriptManager.h>
#include<CommonManager/ActionManager.h>
#include<CommonManager/ListenerManager.h>
#include<CommonManager/AddressManager.h>
#include<CommonCoroutine/CoroutineManager.h>
#include<CommonCore/ManagerFactory.h>

namespace SoEasy
{
	TYPE_REFLECTION(TimerManager);
	TYPE_REFLECTION(ActionManager);
	TYPE_REFLECTION(ScriptManager);
	TYPE_REFLECTION(AddressManager);
	TYPE_REFLECTION(NetWorkManager);
	TYPE_REFLECTION(ListenerManager);
	TYPE_REFLECTION(CoroutineManager);


}



#ifdef _WIN32
#pragma comment(lib,"lua53.lib")
#pragma comment(lib,"Common.lib")
#pragma comment(lib,"libprotobufd.lib")
#endif

using namespace SoEasy;
int main(int argc, char ** argv)
{
	std::string path = "./Config/SrvConfig.json";
	if (argc == 2)
	{
		path = argv[1];
	}
	ManagerFactory managerFactory;
	managerFactory.RegisterManager<TimerManager>();
	managerFactory.RegisterManager<ScriptManager>();
	managerFactory.RegisterManager<ActionManager>();
	managerFactory.RegisterManager<AddressManager>();
	managerFactory.RegisterManager<NetWorkManager>();
	managerFactory.RegisterManager<ListenerManager>();
	managerFactory.RegisterManager<CoroutineManager>();


	Applocation applocation("CenterServer", managerFactory, path);
	
	return applocation.Run();
}
