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
#include <Service/ServiceCenter.h>

#include <Service/MysqlProxy.h>
using namespace Sentry;

int main(int argc, char **argv)
{
	__REGISTER_COMPONENT__(ProxyManager);
	__REGISTER_COMPONENT__(TimerComponent);
	__REGISTER_COMPONENT__(ServiceMgrComponent);
	__REGISTER_COMPONENT__(SceneRedisComponent);
	__REGISTER_COMPONENT__(SceneMysqlComponent);
	__REGISTER_COMPONENT__(SceneActionComponent);
	__REGISTER_COMPONENT__(SceneScriptComponent);
	__REGISTER_COMPONENT__(SceneProtocolComponent);
	
	__REGISTER_COMPONENT__(SceneTaskComponent);
	__REGISTER_COMPONENT__(CoroutineComponent);
	__REGISTER_COMPONENT__(ServiceNodeComponent);
	__REGISTER_COMPONENT__(SceneListenComponent);
	__REGISTER_COMPONENT__(SceneSessionComponent);
	__REGISTER_COMPONENT__(SceneNetProxyComponent);
	
	__REGISTER_COMPONENT__(MysqlProxy);
	__REGISTER_COMPONENT__(LoginService);
	__REGISTER_COMPONENT__(ServiceCenter);
	__REGISTER_COMPONENT__(ClusterService);
	
    std::string serverName = argc == 3 ? argv[1] : "server";
    std::string configPath = argc == 3 ? argv[2] : "./Config/";

    App app(serverName, configPath);

    return app.Run();
}