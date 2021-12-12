#pragma once

#include<Service/ProtoServiceComponent.h>
using namespace com;
namespace GameKeeper
{
    class ProtoRpcComponent;

    class CoroutineComponent;

    class NodeProxyComponent;

    class LocalHostService : public ProtoServiceComponent, public IStart
    {
    public:
		LocalHostService() = default;

        ~LocalHostService() override = default;

    public:
        bool Awake() final;

        void OnStart() final;
        
        bool LateAwake() final;

		int GetPriority() final { return 1000; }
    private:
        XCode Hotfix();
        XCode ReLoadConfig();
        XCode Del(const com::Int32Data &node);
		XCode Add(const s2s::NodeInfo & nodeInfo);
    private:
		int mAreaId;
		int mNodeId;
        std::string mToken;
        long long mOpenTime;
		std::string mNodeName;
        std::string mGroupName;
        NodeProxyComponent * mNodeComponent;
    };
}