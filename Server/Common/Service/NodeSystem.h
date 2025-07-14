//
// Created by zmhy0073 on 2022/10/8.
//

#ifndef APP_INNERSERVICE_H
#define APP_INNERSERVICE_H

#include"Message/com/com.pb.h"
#include"Rpc/Service/RpcService.h"

namespace acs
{
	class NodeSystem final : public RpcService
    {
    public:
        NodeSystem();
        ~NodeSystem() final = default;
    private:
        int Hotfix();
		int Shutdown();
		int LoadConfig();
		int Ping(long long id);
		int Find(const rpc::Message & request); //查找新加入的节点
		int RunInfo(json::w::Document & response); // 获取运行信息
	private:
		int Del(const rpc::Message & request); //服务器退出
		int Add(const json::r::Document & request); //新服务器加入
	private:
		bool OnInit() final;
	private:
		class NodeComponent * mNode;
    };
}


#endif //APP_INNERSERVICE_H
