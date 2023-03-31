//
// Created by zmhy0073 on 2022/10/25.
//

#ifndef APP_LOCATIONSERVICE_H
#define APP_LOCATIONSERVICE_H
#include"Message/s2s.pb.h"
#include"Rpc/Service/PhysicalRpcService.h"
namespace Sentry
{
	class Registry final : public PhysicalRpcService, public ISecondUpdate
    {
    public:
        Registry();
    public:
		bool Awake() final;
		bool OnInit() final;
        bool OnStart() final;
    private:
        int Ping(const Rpc::Packet & head);
        int Query(const com::type::string& request, s2s::server::list& response);
		int Register(const std::string & address,const s2s::server::info & request);
        int UnRegister(const std::string& address, const com::type::string& request);
    private:
		void OnSecondUpdate(int tick) final;
	 private:
        int mIndex;
		std::string mTable;
        class NodeMgrComponent * mNodeComponent;
        class InnerNetComponent* mInnerComponent;
#ifdef __ENABLE_MYSQL__
		class MysqlDBComponent * mMysqlComponent;
#else
		int mDatabaseIndex;
		class SqliteComponent * mSqliteComponent;
#endif
        std::unordered_set<std::string> mRegistryServers;
    };
}


#endif //APP_LOCATIONSERVICE_H
