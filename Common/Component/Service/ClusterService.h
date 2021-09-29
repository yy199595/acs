#pragma once

#include<Service/LocalServiceComponent.h>
using namespace com;
namespace Sentry
{
    class ServiceMgrComponent;

    class CoroutineComponent;

    class ServiceNodeComponent;

    class ClusterService : public LocalServiceComponent
    {
    public:
        ClusterService() {}

        ~ClusterService() {}

    public:
        bool Awake() final;

        void Start() final;

		int GetPriority() final { return 1000; }
    private:
        XCode Del(const com::Int32Data &node);
		XCode Add(const s2s::NodeInfo & nodeInfo);
    private:
        short mAreaId;
        short mNodeId;
        ServiceNodeComponent * mNodeComponent;
    };
}