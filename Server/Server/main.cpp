#include<Core/Applocation.h>
#include<Core/ManagerFactory.h>
#include<Manager/ScriptManager.h>
#include<Manager/TimerManager.h>
#include<Manager/ActionManager.h>
#include<Manager/AddressManager.h>
#include<Manager/NetWorkManager.h>
#include<Manager/CommandManager.h>
#include<Manager/DownLoadManager.h>
#include<Manager/ListenerManager.h>
#include<Manager/MysqlManager.h>
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{
	TYPE_REFLECTION(MysqlManager);
	TYPE_REFLECTION(TimerManager);
	TYPE_REFLECTION(ScriptManager);
	TYPE_REFLECTION(ActionManager);
	TYPE_REFLECTION(AddressManager);
	TYPE_REFLECTION(NetWorkManager);
	TYPE_REFLECTION(CommandManager);
	TYPE_REFLECTION(DownLoadManager);
	TYPE_REFLECTION(ListenerManager);
	TYPE_REFLECTION(CoroutineManager);
}

using namespace SoEasy;
#ifdef _WIN32
#pragma comment(lib,"lua53.lib")
#pragma comment(lib,"Common.lib")
#pragma comment(lib,"hiredis.lib")
#pragma comment(lib,"libmysql.lib")
#pragma comment(lib,"DataBase.lib")
#pragma comment(lib,"ServerData.lib")
#pragma comment(lib,"libprotobufd.lib")
#endif
int main(int argc, char ** argv)
{
	ManagerFactory factory;
	factory.RegisterManager<MysqlManager>();
	factory.RegisterManager<TimerManager>();
	factory.RegisterManager<ScriptManager>();
	factory.RegisterManager<ActionManager>();
	factory.RegisterManager<AddressManager>();
	factory.RegisterManager<NetWorkManager>();
	factory.RegisterManager<CommandManager>();
	factory.RegisterManager<DownLoadManager>();
	factory.RegisterManager<ListenerManager>();
	factory.RegisterManager<CoroutineManager>();


	Applocation app("Server", factory, "./Config/SrvConfig.json");

	return app.Run();
}