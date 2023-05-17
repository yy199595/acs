//
// Created by yjz on 2023/3/2.
//

#ifndef _LOG_H_
#define _LOG_H_
#include"Message/s2s/s2s.pb.h"
#include"Rpc/Service/RpcService.h"
namespace Tendo
{
	class Log final : public RpcService
	{
	 public:
		Log() = default;
	 private:
		int Login(const s2s::log::login & request);
	 protected:
		bool OnInit() final;
	};
}

#endif //_LOG_H_
