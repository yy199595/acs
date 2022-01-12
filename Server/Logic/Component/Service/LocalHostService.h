#pragma once

#include"Component/ServiceBase/ServiceComponent.h"
using namespace com;
namespace GameKeeper
{
    class RpcComponent;

    class TaskComponent;

    class RpcNodeComponent;

    class LocalHostService : public ServiceComponent, public IStart, public ILoadData
    {
    public:
		LocalHostService() = default;

        ~LocalHostService() override = default;

    public:
        bool Awake() final;

        void OnStart() final;
        
        bool LateAwake() final;

        void OnLoadData() final;

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
        std::string mCenterIp;
        unsigned short mCenterPort;
        RpcNodeComponent * mNodeComponent;
    };
}