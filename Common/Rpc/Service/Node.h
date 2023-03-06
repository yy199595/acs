//
// Created by zmhy0073 on 2022/10/8.
//

#ifndef APP_INNERSERVICE_H
#define APP_INNERSERVICE_H
#include"PhysicalService.h"
#include"google/protobuf/wrappers.pb.h"
namespace Sentry
{
    class Node : public PhysicalService
    {
    public:
        Node() = default;
        ~Node() = default;
    private:
		int Ping(const Rpc::Head & head);
		int Stop();
		int Hotfix();
		int LoadConfig();
		int Join(const s2s::server::info & request); //新服务器加入
		int Exit(const s2s::server::info & response); //新服务器加入
		int RunInfo(google::protobuf::StringValue & response); // 获取运行信息
	 private:
        void Init() final;
        bool OnStart() final;
        bool OnClose() final { return false; }
    private:
        class NodeMgrComponent * mNodeComponent;
    };
}


#endif //APP_INNERSERVICE_H
