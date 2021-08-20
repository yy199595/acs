#include <Core/Applocation.h>
#include <Object/ReflectHelper.h>
#include <Manager/ActionManager.h>
#include <Manager/ScriptManager.h>
#include <Timer/TimerManager.h>

#include <Coroutine/CoroutineManager.h>
#include <Manager/ListenerManager.h>
#include <Manager/MysqlManager.h>
#include <Manager/NetProxyManager.h>
#include <Manager/NetSessionManager.h>
#include <Manager/ProxyManager.h>
#include <Manager/RedisManager.h>
#include <Manager/ServiceManager.h>
#include <Manager/ServiceNodeManager.h>
#include <Manager/ThreadTaskManager.h>
#include <Manager/ProtocolManager.h>
#include <Manager/LoginService.h>
#include <Service/ClusterService.h>
#include <Service/ServiceRegistry.h>

#include <Service/MysqlProxy.h>
using namespace Sentry;
#include<Coroutine/Context/context.h>

int main(int argc, char **argv)
{
    ReflectHelper<Manager>::Register<ProxyManager>("ProxyManager");
    ReflectHelper<Manager>::Register<RedisManager>("RedisManager");
    ReflectHelper<Manager>::Register<MysqlManager>("MysqlManager");
    ReflectHelper<Manager>::Register<TimerManager>("TimerManager");
    ReflectHelper<Manager>::Register<ActionManager>("ActionManager");
    ReflectHelper<Manager>::Register<ScriptManager>("ScriptManager");
    ReflectHelper<Manager>::Register<ServiceManager>("ServiceManager");
    ReflectHelper<Manager>::Register<ProtocolManager>("ProtocolManager");


    ReflectHelper<Manager>::Register<ListenerManager>("ListenerManager");
    ReflectHelper<Manager>::Register<CoroutineManager>("CoroutineManager");
    ReflectHelper<Manager>::Register<ServiceNodeManager>("ServiceNodeManager");
    ReflectHelper<Manager>::Register<NetProxyManager>("NetProxyManager");
    ReflectHelper<Manager>::Register<NetSessionManager>("NetSessionManager");
    ReflectHelper<Manager>::Register<ThreadTaskManager>("ThreadTaskManager");


    ReflectHelper<LocalService>::Register<MysqlProxy>("MysqlProxy");
    ReflectHelper<LocalService>::Register<LoginService>("LoginService");
    ReflectHelper<LocalService>::Register<ClusterService>("ClusterService");
    ReflectHelper<LocalService>::Register<ServiceRegistry>("ServiceRegistry");

    std::string serverName = argc == 3 ? argv[1] : "Server";
    std::string configPath = argc == 3 ? argv[2] : "./Config/ServerConfig.json";

    Applocation app(serverName, configPath);

    return app.Run();
}