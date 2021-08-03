#pragma once

#include"LocalService.h"
#include<Protocol/com.pb.h>
#include<Protocol/s2s.pb.h>

using namespace com;
namespace Sentry
{
    class ServiceManager;

    class CoroutineManager;

    class ServiceNodeManager;

    class ClusterService : public LocalService
    {
    public:
        ClusterService() {}

        ~ClusterService() {}

    public:
        bool OnInit() final;

        void OnInitComplete() final;

    private:
        void StarRegisterNode();

    private:
        XCode DelNode(long long, const Int32Data &node);

        XCode AddNode(long long, const s2s::NodeData_NodeInfo &nodeInfo);

    private:
        short mAreaId;
        short mNodeId;
        std::string mQueryAddress;
        std::string mListenAddress;
        ServiceManager *mServiceManager;
        ServiceNodeManager *mServiceNodeManager;
    };
}