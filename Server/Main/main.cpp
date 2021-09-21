#include <Core/App.h>
#include <Scene/ActionComponent.h>
#include <Scene/LuaScriptComponent.h>
#include <Timer/TimerComponent.h>

#include <Coroutine/CoroutineComponent.h>
#include <Scene/ListenerComponent.h>
#include <Scene/MysqlComponent.h>
#include <Scene/NetProxyComponent.h>
#include <Scene/NetSessionComponent.h>
#include <Scene/ProxyManager.h>
#include <Scene/RedisComponent.h>

#include <Service/LuaServiceMgrComponent.h>
#include <Service/ServiceMgrComponent.h>
#include <Service/ServiceNodeComponent.h>
#include <Scene/TaskComponent.h>
#include <Scene/ProtocolComponent.h>
#include <Service/LoginService.h>
#include <Service/ClusterService.h>
#include <Service/CenterService.h>

#include <Service/MysqlService.h>
using namespace Sentry;
int main(int argc, char **argv)
{
	__REGISTER_COMPONENT__(ProxyManager);
	__REGISTER_COMPONENT__(TimerComponent);
	__REGISTER_COMPONENT__(ServiceMgrComponent);
	__REGISTER_COMPONENT__(RedisComponent);
	__REGISTER_COMPONENT__(MysqlComponent);
	__REGISTER_COMPONENT__(ActionComponent);
	__REGISTER_COMPONENT__(LuaScriptComponent);
	__REGISTER_COMPONENT__(ProtocolComponent);
	
	__REGISTER_COMPONENT__(TaskComponent);
	__REGISTER_COMPONENT__(CoroutineComponent);
	__REGISTER_COMPONENT__(ServiceNodeComponent);
	__REGISTER_COMPONENT__(ListenerComponent);
	__REGISTER_COMPONENT__(NetSessionComponent);
	__REGISTER_COMPONENT__(NetProxyComponent);

	__REGISTER_COMPONENT__(LuaServiceMgrComponent);
	
	__REGISTER_COMPONENT__(MysqlService);
	__REGISTER_COMPONENT__(LoginService);
	__REGISTER_COMPONENT__(CenterService);
	__REGISTER_COMPONENT__(ClusterService);
	
    std::string serverName = argc == 3 ? argv[1] : "server";
    std::string configPath = argc == 3 ? argv[2] : "./Config/";

    App app(serverName, configPath);

    return app.Run();
}