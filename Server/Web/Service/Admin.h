//
// Created by zmhy0073 on 2022/8/29.
//

#ifndef APP_ADMIN_H
#define APP_ADMIN_H
#include"Http/Service/HttpService.h"
namespace acs
{
	//运维服务
    class Admin final : public HttpService
    {
    public:
        Admin();
        ~Admin() final = default;
    private:
		bool Awake() final;
        bool OnInit() final;
    private:
		int Hello(http::Response & response);
		int Stop(const http::FromContent & request, json::w::Value & response);
		int Ping(const http::FromContent & request, json::w::Value & response);
		int Hotfix(const http::FromContent & request, json::w::Value & response);
		int Info(const http::FromContent & request, json::w::Document & response);
		int AllInfo(const http::FromContent & request, json::w::Document & response);
		int RpcInterface(const http::FromContent & request, json::w::Document & response);
		int HttpInterface(const http::FromContent & request, json::w::Document & response);
	private:
		int Register(const json::r::Document & request);
		int Update(const http::Request & request, http::Response &);
		int Login(const http::Request & request, json::w::Document & response);
		int List(const http::FromContent & request, json::w::Document & response);
		int Remove(const http::FromContent & request, json::w::Document & response);
	private:
		void AddRpcData(json::w::Value & response, const RpcMethodConfig * config);
	private:
		class ProtoComponent * mProto;
		class AdminComponent * mAdmin;
		class ActorComponent * mActorComponent;
	};
}


#endif //APP_ADMIN_H
