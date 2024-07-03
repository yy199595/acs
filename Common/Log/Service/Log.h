//
// Created by yjz on 2023/3/2.
//

#ifndef APP_LOG_H
#define APP_LOG_H
#include"Message/s2s/s2s.pb.h"
#include"Rpc/Service/RpcService.h"
namespace joke
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

#endif //APP_LOG_H
