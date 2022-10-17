//
// Created by yjz on 2022/6/5.
//

#ifndef _SERVICEAGENT_H_
#define _SERVICEAGENT_H_
#include"RpcService.h"
namespace Sentry
{
	class ServiceWrapper : public RpcService
	{
	 public:
		ServiceWrapper() = default;
		~ServiceWrapper() = default;
	 private:
		bool Start() final { return false;}
		bool Close() final { return false;}
		void WaitAllMessageComplete() final { };
		bool IsStartService() final { return false; }
    private:
		XCode Invoke(const std::string& name, std::shared_ptr<Rpc::Data> message) final;
	};
}

#endif //_SERVICEAGENT_H_
