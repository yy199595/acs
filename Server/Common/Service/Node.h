//
// Created by zmhy0073 on 2022/10/8.
//

#ifndef APP_INNERSERVICE_H
#define APP_INNERSERVICE_H

#include"Message/com/com.pb.h"
#include"Rpc/Service/RpcService.h"

namespace joke
{
	class Node final : public RpcService
    {
    public:
        Node() = default;
        ~Node() final = default;
    private:
        int Hotfix();
		int Shutdown();
		int LoadConfig();
		int Ping(long long id);
		int Find(com::type::json & response);
		int Add(const com::type::json & request); //新服务器加入
        int Del(const com::type::int64 & request); //服务器退出
        int RunInfo(com::type::json & response); // 获取运行信息
		int SetLogLevel(const com::type::int32 & request);
	private:
		bool OnInit() final;
	private:
		class ActorComponent * mActComponent;
    };
}


#endif //APP_INNERSERVICE_H
