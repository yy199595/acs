﻿#include "Entity/Actor/App.h"
#include "Timer/Component/TimerComponent.h"
#include "Async/Component/CoroutineComponent.h"
#include "Lua/Component/LuaComponent.h"

#include "Server/Component/ThreadComponent.h"
#include "Proto/Component/ProtoComponent.h"

#include "Rpc/Component/InnerTcpComponent.h"
#include "Rpc/Component/OuterTcpComponent.h"
#include "Rpc/Component/DispatchComponent.h"

#include "Http/Component/HttpComponent.h"
#include "Http/Component/HttpWebComponent.h"
#include "Entity/Component/ActorComponent.h"

#include "Server/Component/ConfigComponent.h"
#include "Cluster/Component/LaunchComponent.h"
#include "Log/Service/Log.h"
#include "Gate/Service/GateSystem.h"
#include "Common/Service/NodeSystem.h"
#include "Common/Service/LoginSystem.h"
#include "Gate/Component/GateComponent.h"

#include "Mongo/Service/MongoDB.h"
#include "Mongo/Component/MongoDBComponent.h"
#include "Mongo/Component/MongoComponent.h"

#include "Redis/Component/RedisSubComponent.h"

#include "WX/Component/WeChatComponent.h"

#include "Client/Component/TcpClientComponent.h"
#include "Web/Service/LogMgr.h"
#include "Http/Service/ResourceMgr.h"

#include "Record/Component/RecordComponent.h"
#include "Quick/Component/QuickComponent.h"

#include "Redis/Component/DelayQueueComponent.h"
#include "WX/Service/WeChat.h"
#include "Http/Component/NotifyComponent.h"

#include "WX/Component/WXComplaintComponent.h"

#include "Watch/Service/Watch.h"
#include "Watch/Component/WatchComponent.h"

#include "Udp/Component/UdpComponent.h"
#include "Kcp/Component/KcpComponent.h"

#include "WebSocket/Component/InnerWebSocketComponent.h"
#include "WebSocket/Component/OuterWebSocketComponent.h"

#include "Mysql/Service/MysqlDB.h"
#include "Mysql/Component/MysqlDBComponent.h"
#include "Mysql/Component/MysqlComponent.h"


#include "Core/System/System.h"
#include "Redis/Component/RedisComponent.h"
#include "Sqlite/Component/SqliteComponent.h"
#include "Log/Component/LoggerComponent.h"

#include "Server/Config/ServerConfig.h"
#include "Router/Component/RouterComponent.h"

#include "Example/Service/ChatSystem.h"


#include "Upload/Service/FileUpload.h"

#include "Oss/Component/OssComponent.h"
#include "WX/Component/WXNoticeComponent.h"

#include "Quick/Service/QuickSDK.h"

#include "Event/Service/EventSystem.h"

#include "Client/Component/WsClientComponent.h"
#include "Web/Service/Admin.h"
#include "Pgsql/Component/PgsqlDBComponent.h"
#include "Pgsql/Service/PgsqlDB.h"

#include "Mongo/Service/MongoBackup.h"
#include "Telnet/Component/TelnetComponent.h"
using namespace acs;

void RegisterComponent()
{
	REGISTER_COMPONENT(TimerComponent);
	REGISTER_COMPONENT(ProtoComponent);
	REGISTER_COMPONENT(ThreadComponent);
	REGISTER_COMPONENT(ActorComponent);
	REGISTER_COMPONENT(CoroutineComponent);

	REGISTER_COMPONENT(LaunchComponent);
	REGISTER_COMPONENT(RouterComponent);
	REGISTER_COMPONENT(InnerTcpComponent);
	REGISTER_COMPONENT(DispatchComponent);
	REGISTER_COMPONENT(ConfigComponent);

	REGISTER_COMPONENT(GateComponent);
	REGISTER_COMPONENT(OuterTcpComponent);

	REGISTER_COMPONENT(RedisSubComponent);
	REGISTER_COMPONENT(RedisComponent);
	REGISTER_COMPONENT(SqliteComponent);
	REGISTER_COMPONENT(MongoComponent);
	REGISTER_COMPONENT(MongoDBComponent);

	REGISTER_COMPONENT(MysqlDBComponent);
	REGISTER_COMPONENT(MysqlComponent);

	REGISTER_COMPONENT(LuaComponent);
	REGISTER_COMPONENT(HttpComponent);
	REGISTER_COMPONENT(LoggerComponent);
	REGISTER_COMPONENT(HttpWebComponent);
	REGISTER_COMPONENT(TcpClientComponent);
	REGISTER_COMPONENT(ListenerComponent);

	REGISTER_COMPONENT(RecordComponent);

	REGISTER_COMPONENT(DelayQueueComponent);
	REGISTER_COMPONENT(WXNoticeComponent);

	REGISTER_COMPONENT(NotifyComponent);

	REGISTER_COMPONENT(WatchComponent);
	REGISTER_COMPONENT(UdpComponent);
	REGISTER_COMPONENT(KcpComponent);
	REGISTER_COMPONENT(QuickComponent);
	REGISTER_COMPONENT(InnerWebSocketComponent);
	REGISTER_COMPONENT(OuterWebSocketComponent);
	REGISTER_COMPONENT(WsClientComponent);
#ifdef __ENABLE_OPEN_SSL__
	REGISTER_COMPONENT(WeChatComponent);
	REGISTER_COMPONENT(WXComplaintComponent);
#endif
	REGISTER_COMPONENT(OssComponent);

	REGISTER_COMPONENT(PgsqlDBComponent);

	REGISTER_COMPONENT(TelnetComponent);
}

void RegisterAll()
{
	RegisterComponent();

	REGISTER_COMPONENT(Log);
	REGISTER_COMPONENT(Admin);
	REGISTER_COMPONENT(GateSystem);
	REGISTER_COMPONENT(NodeSystem);
	REGISTER_COMPONENT(LoginSystem);

	REGISTER_COMPONENT(MongoDB);
	REGISTER_COMPONENT(MysqlDB);
	REGISTER_COMPONENT(PgsqlDB);

	REGISTER_COMPONENT(MongoBackup);

#ifdef __ENABLE_OPEN_SSL__
	REGISTER_COMPONENT(WeChat);
#endif
	REGISTER_COMPONENT(ChatSystem);
	REGISTER_COMPONENT(LogMgr);
	REGISTER_COMPONENT(ResourceMgr);

	REGISTER_COMPONENT(FileUpload);

	REGISTER_COMPONENT(QuickSDK);

	REGISTER_COMPONENT(Watch);
	REGISTER_COMPONENT(EventSystem);
}

int main(int argc, char** argv)
{
#ifdef __OS_WIN__
	system("chcp 65001");
#endif

	int id = 0;
	RegisterAll();
	std::string path;
	ServerConfig config;
	os::System::Init(argc, argv);
	os::System::GetEnv("ID", id);
	os::System::GetEnv("CONFIG", path);
	{
		if(!config.LoadConfig(path))
		{
			return -1;
		}
		return (new App(id, config))->Run();
	}
}
