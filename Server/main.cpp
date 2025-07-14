
//#include "vld.h"
#include "Log/Common/Rang.h"
#include "Entity/Actor/App.h"
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
#include "Node/Component/NodeComponent.h"

#include "Server/Component/ConfigComponent.h"
#include "Cluster/Component/LaunchComponent.h"
#include "Log/Service/Log.h"
#include "Gate/Service/GateSystem.h"
#include "Common/Service/NodeSystem.h"
#include "Common/Service/LoginSystem.h"
#include "Gate/Component/GateComponent.h"

#include "Mongo/Component/MongoDBComponent.h"

#include "Redis/Component/RedisSubComponent.h"

#include "WX/Component/WeChatComponent.h"

#include "Client/Component/TcpClientComponent.h"
#include "Web/Service/LogMgr.h"
#include "Http/Service/ResourceMgr.h"

#include "Record/Component/RecordComponent.h"
#include "Quick/Component/QuickComponent.h"

#include "Redis/Component/DelayMQComponent.h"
#include "WX/Service/WeChat.h"
#include "Http/Component/NotifyComponent.h"

#include "WX/Component/WXComplaintComponent.h"

#include "Watch/Service/Watch.h"
#include "Watch/Component/WatchComponent.h"

#include "Udp/Component/UdpComponent.h"
#include "Kcp/Component/KcpComponent.h"

#include "WebSocket/Component/InnerWebSocketComponent.h"
#include "WebSocket/Component/OuterWebSocketComponent.h"

#include "Mysql/Component/MysqlDBComponent.h"

#include "Core/System/System.h"
#include "Redis/Component/RedisComponent.h"
#include "Sqlite/Component/SqliteComponent.h"
#include "Log/Component/LoggerComponent.h"

#include "Router/Component/RouterComponent.h"

#include "Example/Service/ChatSystem.h"


#include "Upload/Service/FileUpload.h"

#include "AliCloud/Component/AliOssComponent.h"
#include "WX/Component/WXNoticeComponent.h"

#include "Quick/Service/QuickSDK.h"

#include "PubSub/Service/PubSubSystem.h"
#include "PubSub/Component/PubSubComponent.h"

#include "Client/Component/WsClientComponent.h"
#include "Web/Service/Admin.h"
#include "Pgsql/Component/PgsqlDBComponent.h"

#include "Mongo/Service/MongoBackup.h"
#include "Telnet/Component/TelnetComponent.h"
#include "Common/Component/PlayerComponent.h"

#include "Registry/Service/RegistryService.h"
#include "Mongo/Service/MongoReadProxy.h"
#include "Mongo/Service/MongoWriteProxy.h"
#include "Mysql/Service/MysqlReadProxy.h"
#include "Mysql/Service/MysqlWriteProxy.h"
#include "Pgsql/Service/PgsqlReadProxy.h"
#include "Pgsql/Service/PgsqlWriteProxy.h"
#include "Mysql/Service/MysqlBackup.h"

#include "MeiliSearch/Component/MeiliSearchComponent.h"
using namespace acs;



void RegisterComponent()
{
	REGISTER_COMPONENT(TimerComponent);
	REGISTER_COMPONENT(ProtoComponent);
	REGISTER_COMPONENT(ThreadComponent);
	REGISTER_COMPONENT(NodeComponent);
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
	REGISTER_COMPONENT(MongoDBComponent);

	REGISTER_COMPONENT(MysqlDBComponent);

	REGISTER_COMPONENT(LuaComponent);
	REGISTER_COMPONENT(HttpComponent);
	REGISTER_COMPONENT(LoggerComponent);
	REGISTER_COMPONENT(HttpWebComponent);
	REGISTER_COMPONENT(TcpClientComponent);
	REGISTER_COMPONENT(ListenerComponent);

	REGISTER_COMPONENT(RecordComponent);

	REGISTER_COMPONENT(DelayMQComponent);
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
	REGISTER_COMPONENT(AliOssComponent);

	REGISTER_COMPONENT(PgsqlDBComponent);

	REGISTER_COMPONENT(TelnetComponent);

	REGISTER_COMPONENT(PlayerComponent);

	REGISTER_COMPONENT(PubSubComponent);
	REGISTER_COMPONENT(MeiliSearchComponent);
}

void RegisterAll()
{
	RegisterComponent();

	REGISTER_COMPONENT(Log);
	REGISTER_COMPONENT(Admin);
	REGISTER_COMPONENT(GateSystem);
	REGISTER_COMPONENT(NodeSystem);
	REGISTER_COMPONENT(LoginSystem);
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
	REGISTER_COMPONENT(PubSubSystem);

	REGISTER_COMPONENT(RegistryService);

	REGISTER_COMPONENT(MongoReadProxy);
	REGISTER_COMPONENT(MongoWriteProxy);

	REGISTER_COMPONENT(MysqlBackup);
	REGISTER_COMPONENT(MysqlReadProxy);
	REGISTER_COMPONENT(MysqlWriteProxy);

	REGISTER_COMPONENT(PgsqlReadProxy);
	REGISTER_COMPONENT(PgsqlWriteProxy);
}
#include "Core/Bloom/BloomFilter.h"
int main(int argc, char** argv)
{
	bloom::Filter<> bloom_component;
	bool result1 = bloom_component.Has("192.168.1.2");
	bloom_component.Set("192.168.1.2");
	bool result2 = bloom_component.Has("192.168.1.2");
	bloom_component.Del("192.168.1.2");
	bool result3 = bloom_component.Has("192.168.1.2");

#ifdef __OS_WIN__
	SetConsoleOutputCP(CP_UTF8);
	setWinTermMode(rang::winTerm::Auto);
#endif
	int id = 0;
	RegisterAll();
	std::string cmd, name;
	os::System::Init(argc, argv);
	os::System::GetAppEnv("id", id);
	os::System::GetAppEnv("name", name);
	if(!os::System::GetAppEnv("cmd", cmd))
	{
		return (new App(id, name))->Run();
	}
	return (new App(id, name))->Run(cmd);
}
