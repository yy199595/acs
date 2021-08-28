#include <Core/App.h>
#include <Scene/SceneActionComponent.h>
#include <Scene/SceneScriptComponent.h>
#include <Timer/TimerComponent.h>

#include <Coroutine/CoroutineComponent.h>
#include <Scene/SceneListenComponent.h>
#include <Scene/SceneMysqlComponent.h>
#include <Scene/SceneNetProxyComponent.h>
#include <Scene/SceneSessionComponent.h>
#include <Scene/ProxyManager.h>
#include <Scene/SceneRedisComponent.h>


#include <Service/ServiceMgrComponent.h>
#include <Service/ServiceNodeComponent.h>
#include <Scene/SceneTaskComponent.h>
#include <Scene/SceneProtocolComponent.h>
#include <Service/LoginService.h>
#include <Service/ClusterService.h>
#include <Service/ServiceRegistry.h>

#include <Service/MysqlProxy.h>
using namespace Sentry;
#include<Coroutine/Context/context.h>

int main(int argc, char **argv)
{
	ComponentHelper::Add<ProxyManager>("ProxyManager");
    ComponentHelper::Add<SceneRedisComponent>("SceneRedisComponent");
    ComponentHelper::Add<SceneMysqlComponent>("SceneMysqlComponent");
    ComponentHelper::Add<TimerComponent>("TimerComponent");
    ComponentHelper::Add<SceneActionComponent>("SceneActionComponent");
    ComponentHelper::Add<SceneScriptComponent>("SceneScriptComponent");
    ComponentHelper::Add<SceneProtocolComponent>("SceneProtocolComponent");
	ComponentHelper::Add<ServiceMgrComponent>("ServiceMgrComponent");


    ComponentHelper::Add<SceneListenComponent>("SceneListenComponent");
    ComponentHelper::Add<CoroutineComponent>("CoroutineComponent");
    ComponentHelper::Add<ServiceNodeComponent>("ServiceNodeComponent");
    ComponentHelper::Add<SceneNetProxyComponent>("SceneNetProxyComponent");
    ComponentHelper::Add<SceneSessionComponent>("SceneSessionComponent");
    ComponentHelper::Add<SceneTaskComponent>("SceneTaskComponent");


    ComponentHelper::Add<MysqlProxy>("MysqlProxy");
    ComponentHelper::Add<LoginService>("LoginService");
    ComponentHelper::Add<ClusterService>("ClusterService");
    ComponentHelper::Add<ServiceRegistry>("ServiceRegistry");

    std::string serverName = argc == 3 ? argv[1] : "Server";
    std::string configPath = argc == 3 ? argv[2] : "./Config/ServerConfig.json";

    App app(serverName, configPath);

    return app.Run();
}