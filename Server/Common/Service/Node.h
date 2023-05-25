//
// Created by zmhy0073 on 2022/10/8.
//

#ifndef APP_INNERSERVICE_H
#define APP_INNERSERVICE_H

#include"Message/s2s/s2s.pb.h"
#include"Message/com/com.pb.h"
#include"Rpc/Service/RpcService.h"
namespace Tendo
{
	class Node final : public RpcService
    {
    public:
        Node() = default;
        ~Node() = default;
    private:
        int Hotfix();
		int Shutdown();
		int LoadConfig();
		int Ping(const Msg::Packet& packet);
		int Join(const s2s::registry::request& request); //新服务器加入
        int Exit(const s2s::registry::request& request); //服务器退出
        int RunInfo(com::type::string& response); // 获取运行信息
    private:
		bool OnInit() final;
    };
}


#endif //APP_INNERSERVICE_H
