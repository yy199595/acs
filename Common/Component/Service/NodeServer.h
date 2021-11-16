#pragma once

#include<Service/LocalServiceComponent.h>
using namespace com;
namespace GameKeeper
{
    class RpcRequestComponent;

    class CoroutineComponent;

    class NodeProxyComponent;

    class NodeServer : public LocalServiceComponent
    {
    public:
		NodeServer() = default;

        ~NodeServer() override = default;

    public:
        bool Awake() final;

        void Start() final;

		int GetPriority() final { return 1000; }
    private:
        XCode Del(const com::Int32Data &node);
		XCode Add(const s2s::NodeInfo & nodeInfo);
    private:
        std::string mToken;
        long long mOpenTime;
        std::string mGroupName;
        NodeProxyComponent * mNodeComponent;
    };
}