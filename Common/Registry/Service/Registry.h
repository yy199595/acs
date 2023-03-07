//
// Created by zmhy0073 on 2022/10/25.
//

#ifndef APP_LOCATIONSERVICE_H
#define APP_LOCATIONSERVICE_H
#include"Service/PhysicalService.h"
namespace Sentry
{
    class Registry : public PhysicalService
    {
    public:
        Registry() = default;
    public:
        bool OnStart() final;
        bool OnClose() final;
    private:
        int Ping(const Rpc::Head & head);
        int UnRegister(const com::type::string& request);
        int Query(const com::type::string& request, s2s::server::list& response);
		int Register(const s2s::server::info & request, s2s::server::list & response);
    private:      
        void OnNodeServerError(const std::string& address);
	 private:
        class NodeMgrComponent * mNodeComponent;
        class InnerNetComponent* mInnerComponent;
    };
}


#endif //APP_LOCATIONSERVICE_H
