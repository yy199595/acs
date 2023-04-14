//
// Created by zmhy0073 on 2022/10/25.
//

#ifndef APP_LOCATIONSERVICE_H
#define APP_LOCATIONSERVICE_H
#include"Message/s2s.pb.h"
#include"Rpc/Service/PhysicalRpcService.h"
namespace Tendo
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
        int Query(const s2s::server::query& request, s2s::server::list& response);
		int Register(const std::string & address,const s2s::server::info & request);
        int UnRegister(const std::string& address, const s2s::server::info& request);
    private:
		void OnClose() final { }
		void OnSecondUpdate(int tick) final;
	 private:
		std::string mTable;
        class RedisComponent* mRedisComponent;
        class NodeMgrComponent * mNodeComponent;
        class InnerNetComponent* mInnerComponent;
        std::unordered_set<std::string> mRegistryServers;
    };
}


#endif //APP_LOCATIONSERVICE_H
