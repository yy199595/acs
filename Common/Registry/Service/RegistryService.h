//
// Created by 64658 on 2025/4/9.
//

#ifndef APP_REGISTRYSERVICE_H
#define APP_REGISTRYSERVICE_H
#include "Rpc/Service/RpcService.h"
#include "Yyjson/Object/JsonObject.h"

namespace node
{
	struct Info : public json::Object<Info>
	{
	public:
		int id = 0;
		int sockId = 0;
		std::string name;
		std::string listen;
		long long last_time = 0;
		std::vector<int> addQueue; //通知队列
		std::vector<int> delQueue; //移除队列
	};
	constexpr int TIME_OUT = 5; //超时时间
	constexpr const char * REGISTRY_LIST = "registry_list";
}

namespace acs
{
	class RegistryService : public RpcService, public IStart, public ISecondUpdate
	{
	public:
		RegistryService();
	private:
		bool OnInit() final;
		void OnStart() final;
		void OnSecondUpdate(int tick) noexcept final;
	private:
		int Add(const rpc::Message & request);
		int Del(const rpc::Message & request);
		int Ping(const rpc::Message & request);
		int Find(const rpc::Message & request, rpc::Message & response);
	private:
		void Broadcast(int id);
		node::Info * Find(int id);
		void Broadcast(node::Info & nodeInfo);
	private:
		void NoticeMessage();
		void NoticeMessage(node::Info & nodeInfo);
	private:
		class SqliteComponent * mSqlite;
		class RouterComponent * mRouter;
		std::unordered_map<int, node::Info> mNodeList;
	};

}


#endif //APP_REGISTRYSERVICE_H
