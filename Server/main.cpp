#include "Entity/Actor/App.h"
#include "Timer/Component/TimerComponent.h"
#include "Async/Component/CoroutineComponent.h"
#include "Lua/Component/LuaComponent.h"

#include "Server/Component/ThreadComponent.h"
#include "Proto/Component/ProtoComponent.h"

#include "Rpc/Component/InnerNetComponent.h"
#include "Gate/Component/OuterNetComponent.h"
#include "Rpc/Component/DispatchComponent.h"

#include "Http/Component/HttpComponent.h"
#include "Http/Component/HttpWebComponent.h"
#include "Entity/Component/ActorComponent.h"

#include "Server/Component/ConfigComponent.h"
#include "Cluster/Component/LaunchComponent.h"
#include "Log/Service/Log.h"
#include "Gate/Service/Gate.h"
#include "Common/Service/Node.h"
#include "Web/Service/Admin.h"
#include "Common/Service/Login.h"
#include "Sqlite/Service/SqliteDB.h"
#include "Master/Component/MasterComponent.h"
#include "Gate/Component/GateComponent.h"

#include "Kcp/Component/KcpComponent.h"

#include "Mongo/Service/MongoDB.h"
#include "Mongo/Component/MongoDBComponent.h"
#include "Mongo/Component/MongoComponent.h"

#include "Redis/Component/SubComponent.h"

#include "WX/Component/WeChatComponent.h"

#include "Client/Component/ClientComponent.h"
#include "Web/Service/LogMgr.h"
#include "Http/Service/ResourceMgr.h"


#include "Record/Component/RecordComponent.h"

#include "Redis/Component/DelayQueueComponent.h"
#include "WX/Service/WeChat.h"
#include "Http/Component/GroupNotifyComponent.h"

#include "Operation/Component/OperationComponent.h"

#ifdef __ENABLE_MYSQL__
#include "Mysql/Service/MysqlDB.h"
#include "Mysql/Component/MysqlDBComponent.h"
#include "Mysql/Component/MysqlHelperComponent.h"
#endif

#include "Core/System/System.h"
#include "Redis/Component/RedisComponent.h"
#include "Sqlite/Component/SqliteComponent.h"
#include "Log/Component/LoggerComponent.h"

#include "Master/Service/Master.h"

#include "Server/Config/ServerConfig.h"
#include "Router/Component/RouterComponent.h"

#include "Example/Service/Chat.h"

#include "Upload/Service/FileUpload.h"

#include "Web/Component/AdminComponent.h"
#include "Oss/Component/OssComponent.h"
#include "WX/Component/WXNoticeComponent.h"


#include "Quick/Service/QuickSDK.h"

using namespace joke;

void RegisterComponent()
{
	REGISTER_COMPONENT(TimerComponent);
	REGISTER_COMPONENT(ProtoComponent);
	REGISTER_COMPONENT(ThreadComponent);
	REGISTER_COMPONENT(ActorComponent);
	REGISTER_COMPONENT(CoroutineComponent);

	REGISTER_COMPONENT(KcpComponent);
	REGISTER_COMPONENT(LaunchComponent);
	REGISTER_COMPONENT(RouterComponent);
	REGISTER_COMPONENT(InnerNetComponent);
	REGISTER_COMPONENT(MasterComponent);
	REGISTER_COMPONENT(DispatchComponent);
	REGISTER_COMPONENT(ConfigComponent);

	REGISTER_COMPONENT(GateComponent);
	REGISTER_COMPONENT(OuterNetComponent);

	REGISTER_COMPONENT(SubComponent);
	REGISTER_COMPONENT(RedisComponent);
	REGISTER_COMPONENT(SqliteComponent);
	REGISTER_COMPONENT(MongoComponent);
	REGISTER_COMPONENT(MongoDBComponent);

#ifdef __ENABLE_MYSQL__
	REGISTER_COMPONENT(MysqlDBComponent);
	REGISTER_COMPONENT(MysqlHelperComponent);
#endif

	REGISTER_COMPONENT(LuaComponent);
	REGISTER_COMPONENT(HttpComponent);
	REGISTER_COMPONENT(LoggerComponent);
	REGISTER_COMPONENT(HttpWebComponent);
	REGISTER_COMPONENT(ClientComponent);
	REGISTER_COMPONENT(ListenerComponent);

	REGISTER_COMPONENT(WeChatComponent);

	REGISTER_COMPONENT(AdminComponent);
	REGISTER_COMPONENT(RecordComponent);

	REGISTER_COMPONENT(OssComponent);

	REGISTER_COMPONENT(OperationComponent);
	REGISTER_COMPONENT(DelayQueueComponent);
	REGISTER_COMPONENT(WXNoticeComponent);

	REGISTER_COMPONENT(GroupNotifyComponent);
}

void RegisterAll()
{
	RegisterComponent();

	REGISTER_COMPONENT(Log);
	REGISTER_COMPONENT(Gate);
	REGISTER_COMPONENT(Node);
	REGISTER_COMPONENT(Login);

	REGISTER_COMPONENT(Admin);
	REGISTER_COMPONENT(Master);
	REGISTER_COMPONENT(MongoDB);
	REGISTER_COMPONENT(SqliteDB);
#ifdef __ENABLE_MYSQL__
	REGISTER_COMPONENT(MysqlDB);
#endif

	REGISTER_COMPONENT(WeChat);

	REGISTER_COMPONENT(Chat);
	REGISTER_COMPONENT(LogMgr);
	REGISTER_COMPONENT(ResourceMgr);

	REGISTER_COMPONENT(FileUpload);

	REGISTER_COMPONENT(QuickSDK);
}

int main(int argc, char **argv)
{
//#ifdef __OS_WIN__
//	system("chcp 65001 > nul");
//#endif
	int id = 0;
	RegisterAll();
	std::string path;
	ServerConfig config;
	System::Init(argc, argv);
	System::GetEnv("ID", id);
	if (System::GetEnv("CONFIG", path) && config.LoadConfig(path))
	{
#ifdef __OS_WIN__
		std::string title = fmt::format("{} pid={}", config.Name(), getpid());
		SetConsoleTitle(title.c_str());
#endif
		return (new App(id, config))->Run();
	}
	return -1;
}
